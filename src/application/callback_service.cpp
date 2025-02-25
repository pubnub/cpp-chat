#include "callback_service.hpp"
#include "application/bundles.hpp"
#include "application/chat_service.hpp"
#include "application/dao/membership_dao.hpp"
#include "const_values.hpp"
#include "enums.hpp"
#include "infra/serialization.hpp"
#include "infra/sync.hpp"
#include "nlohmann/json.hpp"
#include "domain/parsers.hpp"
#include "message.hpp"
#include <algorithm>
#include <cstring>
#include <list>
#include <pubnub_helper.h>
#include <pubnub_subscribe_event_listener_types.h>

using json = nlohmann::json;

CallbackService::CallbackService(
        EntityServicesBundle entity_bundle,
        std::weak_ptr<const PresenceService> presence_service,
        ThreadSafePtr<PubNub> pubnub,
        milliseconds wait_interval) :
    pubnub(pubnub),
    thread_run_flag(true),
    chat_service(entity_bundle.chat_service.lock()),
    message_service(entity_bundle.message_service),
    channel_service(entity_bundle.channel_service),
    user_service(entity_bundle.user_service),
    membership_service(entity_bundle.membership_service),
    presence_service(presence_service)
{
    this->callback_thread = std::thread([this, wait_interval]() {
//        while (this->thread_run_flag.load()) {
//            this->resolve_callbacks();
//            this->resolve_timers(wait_interval);
//            // TODO: should we sleep in tests?
//            std::this_thread::sleep_for(wait_interval);
//        }
    });
}

void CallbackService::register_message_callback(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback)
{
    this->callbacks.get_message_callbacks().update_or_insert(channel_id, message_callback);
}

void CallbackService::remove_message_callback(Pubnub::String channel_id)
{
    this->callbacks.get_message_callbacks().remove(channel_id);
}

void CallbackService::register_thread_message_update_callback(Pubnub::String channel_id, std::function<void(Pubnub::ThreadMessage)> thread_message_callback)
{
    this->callbacks.get_thread_message_update_callbacks().update_or_insert(channel_id, thread_message_callback);
}

void CallbackService::remove_thread_message_update_callback(Pubnub::String channel_id)
{
    this->callbacks.get_thread_message_update_callbacks().remove(channel_id);
}

void CallbackService::register_message_update_callback(
        Pubnub::String message_timetoken,
        std::function<void(Pubnub::Message)> message_update_callback
)
{
    this->callbacks.get_message_update_callbacks().update_or_insert(message_timetoken, message_update_callback);
}

void CallbackService::remove_message_update_callback(Pubnub::String message_timetoken)
{
    this->callbacks.get_message_update_callbacks().remove(message_timetoken);
}

void CallbackService::register_channel_callback(Pubnub::String channel_id, std::function<void(Pubnub::Channel)> channel_callback)
{
    this->callbacks.get_channel_callbacks().update_or_insert(channel_id, channel_callback);
}

void CallbackService::remove_channel_callback(Pubnub::String channel_id)
{
    this->callbacks.get_channel_callbacks().remove(channel_id);
}

void CallbackService::register_event_callback(
        Pubnub::String channel_id,
        Pubnub::pubnub_chat_event_type chat_event_type,
        std::function<void(Pubnub::Event)> event_callback
)
{
    //TODO: Storing this in map is not good idea, as someone could listen for 2 types on the same channel. Then only 1 type would work.
    //But it's not causing any issues in MVP, as only 2 types are supported and type REPORT can only be used with Internal Admin Channel
    //In MVP we only support these 2 types.
    this->callbacks.get_event_callbacks().update_or_insert(channel_id, std::make_pair(chat_event_type, event_callback));
}

void CallbackService::remove_event_callback(Pubnub::String channel_id, Pubnub::pubnub_chat_event_type chat_event_type)
{
    this->callbacks.get_event_callbacks().remove(channel_id);
}

void CallbackService::register_user_callback(Pubnub::String user_id, std::function<void(Pubnub::User)> user_callback)
{
    this->callbacks.get_user_callbacks().update_or_insert(user_id, user_callback);
}

void CallbackService::remove_user_callback(Pubnub::String user_id)
{
    this->callbacks.get_user_callbacks().remove(user_id);
}

void CallbackService::register_channel_presence_callback(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback)
{
    this->callbacks.get_channel_presence_callbacks().update_or_insert(channel_id, presence_callback);
}

void CallbackService::remove_channel_presence_callback(Pubnub::String channel_id)
{
    this->callbacks.get_channel_presence_callbacks().remove(channel_id);
}

void CallbackService::register_membership_callback(
        Pubnub::String channel_id,
        Pubnub::String user_id,
        std::function<void(Pubnub::Membership)> membership_callback
)
{
    this->callbacks.get_membership_callbacks().update_or_insert(channel_id, std::make_pair(user_id, membership_callback));
}

void CallbackService::remove_membership_callback(Pubnub::String channel_id)
{
    this->callbacks.get_membership_callbacks().remove(channel_id);
}

void CallbackService::broadcast_messages(std::vector<pubnub_v2_message> messages)
{
    for (auto message : messages) {
        this->broadcast_callbacks_from_message(message);
    }
}

void CallbackService::resolve_callbacks() {
    auto messages = [this]{
        auto guard = this->pubnub->lock();
        return guard->fetch_messages();
    }();

    this->broadcast_messages(messages);
}

void CallbackService::resolve_timers(milliseconds wait_interval) {

}

void CallbackService::broadcast_callbacks_from_message(pubnub_v2_message message)
{
    if(!message.payload.ptr || !message.channel.ptr)
    {
        throw std::runtime_error("received message is invalid");
    }

    Pubnub::String message_string = Pubnub::String(message.payload.ptr, message.payload.size);
    Pubnub::String message_channel_string = Pubnub::String(message.channel.ptr, message.channel.size);

    if(message_string.empty())
    {
        throw std::runtime_error("message is empty");
    }

    // TODO: merge this combination of ifs into something more readable
    if (Parsers::PubnubJson::is_message(message_string)) {
        auto maybe_callback = this->callbacks.get_message_callbacks().get(message_channel_string);
        if (maybe_callback.has_value()) {
            auto parsed_message = Parsers::PubnubJson::to_message(message);
            if (auto service = this->message_service.lock()) {
                maybe_callback.value()(service->create_message_object(parsed_message));
            } else {
                throw std::runtime_error("Message service is not available to call callback");
            }
        }
    }
    
    if (Parsers::PubnubJson::is_channel_update(message_string)) {
        auto maybe_callback = this->callbacks.get_channel_callbacks().get(message_channel_string);
        if (maybe_callback.has_value()) {
            auto parsed_channel = Parsers::PubnubJson::to_channel(message);
            if (auto service = this->channel_service.lock()) {
                maybe_callback.value()(service->create_channel_object(parsed_channel));
            } else {
                throw std::runtime_error("Channel service is not available to call callback");
            }
        }
    }

    if (Parsers::PubnubJson::is_user_update(message_string)) {
        auto maybe_callback = this->callbacks.get_user_callbacks().get(message_channel_string);
        if (maybe_callback.has_value()) {
            auto parsed_user = Parsers::PubnubJson::to_user(message);
            if (auto service = this->user_service.lock()) {
                maybe_callback.value()(service->create_user_object(parsed_user));
            } else {
                throw std::runtime_error("User service is not available to call callback");
            }
        }
    }

    if (Parsers::PubnubJson::is_event(message_string)) {
        auto maybe_callback = this->callbacks.get_event_callbacks().get(message_channel_string);
        if (maybe_callback.has_value()) {
            auto parsed_event = Parsers::PubnubJson::to_event(message);

            Pubnub::pubnub_chat_event_type event_type;
            std::function<void(Pubnub::Event)> callback;
            std::tie(event_type, callback) = maybe_callback.value();

            if (parsed_event.type == event_type) {
                callback(parsed_event);
            }
        }
    }

    if (Parsers::PubnubJson::is_presence(message_string)) {
        Pubnub::String normal_channel_name = message_channel_string;
        auto pnpres_length = strlen("-pnpres");
        normal_channel_name.erase(message_channel_string.length() - pnpres_length, pnpres_length);

        auto maybe_callback = this->callbacks.get_channel_presence_callbacks().get(normal_channel_name);
        if (maybe_callback.has_value()) {
            if (auto service = this->presence_service.lock()) {
                maybe_callback.value()(service->who_is_present(normal_channel_name));
            } else {
                throw std::runtime_error("Presence service is not available to call callback");
            }
        }
    }

    if (Parsers::PubnubJson::is_message_update(message_string)) {
        Pubnub::String message_timetoken = Parsers::PubnubJson::message_update_timetoken(message_string);

        auto parsed_message = this->message_service
            .lock()
            ->create_message_object(Parsers::PubnubJson::to_message_update(message));
        
        auto prefix_length = std::strlen(Pubnub::MESSAGE_THREAD_ID_PREFIX) - 1;
        if (message.channel.size >= prefix_length && std::strncmp(
                    message.channel.ptr,
                    Pubnub::MESSAGE_THREAD_ID_PREFIX,
                    prefix_length
            ) == 0) {
            auto maybe_callback = this->callbacks.get_thread_message_update_callbacks().get(message_timetoken);
            if (maybe_callback.has_value()) {
                const auto timetoken_length = 17;
                const auto underscores = 2;
                const auto channel_padding_length = timetoken_length + prefix_length + underscores;

                auto channel = Pubnub::String(message.channel.ptr, message.channel.size);
                auto parent_channel = channel.substring(prefix_length, channel.length() - channel_padding_length);

                maybe_callback.value()(Pubnub::ThreadMessage(parsed_message, parent_channel));
            }
        } else {       
            auto maybe_callback = this->callbacks.get_message_update_callbacks().get(message_timetoken);
            if (maybe_callback.has_value()) {
                maybe_callback.value()(parsed_message);
            }
        }
    }

    if (Parsers::PubnubJson::is_membership_update(message_string)) {
        auto membership_channel = Parsers::PubnubJson::membership_channel(message_string);

        auto maybe_callback = this->callbacks.get_membership_callbacks().get(membership_channel);
        if (maybe_callback.has_value()) {
            Pubnub::String membership_user;
            std::function<void(Pubnub::Membership)> callback;
            std::tie(membership_user, callback) = maybe_callback.value();

            //Make sure this message is related to the user that we are streaming updates for
            auto user_from_message = Parsers::PubnubJson::membership_user(message_string);
            if(user_from_message == membership_user)
            {
                auto custom_field = Parsers::PubnubJson::membership_from_string(message_string);
                auto memberships = this->membership_service.lock();
                auto channels = this->channel_service.lock();
                auto users = this->user_service.lock();
                if (memberships && channels && users) {
                    Pubnub::Membership membership_obj = memberships->create_membership_object(
                        users->get_user(membership_user),
                        channels->get_channel(membership_channel),
                        custom_field
                    );
                    callback(membership_obj);
                } else {
                    throw std::runtime_error("Membership, channel or user service is not available to call callback");
                }
            }
        }
    }
}

static void notify_error(const Pubnub::String& error_message) {
    // Basically, we cannot throw exceptions in C callbacks, so we just print the error message 
    // and let the program continue because we cannot do anything else.
    // TODO: Logger
    std::cout << "Callback error: " << error_message << std::endl;
}

CCoreCallbackData CallbackService::to_c_message_callback(std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Message)> message_callback) {
    struct CallbackCtx {
        std::weak_ptr<const ChatService> chat_service;
        std::function<void(Pubnub::Message)> message_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_message(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto parsed_message = Parsers::PubnubJson::to_message(message);
                if (auto service = ctx.chat_service.lock()) {
                    ctx.message_callback(service->message_service->create_message_object(parsed_message));
                } else {
                    notify_error("Chat service is not available to call message callback");
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{chat_service, message_callback})
    };
}

CCoreCallbackData CallbackService::to_c_channel_update_callback(Pubnub::Channel base_channel, std::shared_ptr<const ChannelService> channel_service, std::function<void (Pubnub::Channel)> channel_update_callback) {
    struct CallbackCtx {
        Pubnub::Channel base_channel;
        std::shared_ptr<const ChannelService> channel_service;
        std::function<void (Pubnub::Channel)> channel_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_channel_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));
                auto channel = ctx.channel_service->create_channel_object(Parsers::PubnubJson::to_channel(message));
                auto updated = ctx.channel_service->update_channel_with_base(channel, ctx.base_channel);
                ctx.channel_update_callback(updated);
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{base_channel, channel_service, channel_update_callback})
    };
}


CCoreCallbackData CallbackService::to_c_channels_updates_callback(const std::vector<Pubnub::Channel>& channels, std::shared_ptr<const ChannelService> channel_service, std::function<void(std::vector<Pubnub::Channel>)> channel_update_callback) {
    struct CallbackCtx {
        const std::vector<Pubnub::Channel> channels;
        std::shared_ptr<const ChannelService> channel_service;
        std::function<void(std::vector<Pubnub::Channel>)> channel_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_channel_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto channel = ctx.channel_service->create_channel_object(Parsers::PubnubJson::to_channel(message));
                auto stream_channel = std::find_if(
                        ctx.channels.begin(),
                        ctx.channels.end(),
                        [&channel](const Pubnub::Channel& base_channel) {
                            return base_channel.channel_id() == channel.channel_id();
                    });

                ChannelEntity stream_channel_entity = ChannelDAO(stream_channel->channel_data()).to_entity();
                ChannelEntity channel_entity = ChannelDAO(channel.channel_data()).to_entity();
                auto pair = std::make_pair(channel.channel_id(), ChannelEntity::from_base_and_updated_channel(stream_channel_entity, channel_entity));
                auto updated_channel = ctx.channel_service->create_channel_object(pair);

                std::vector<Pubnub::Channel> updated_channels;

                // TODO: is the order important?
                std::copy_if(
                        ctx.channels.begin(),
                        ctx.channels.end(),
                        std::back_inserter(updated_channels),
                        [&channel](const Pubnub::Channel& base_channel) {
                            return base_channel.channel_id() != channel.channel_id();
                    });

                updated_channels.push_back(updated_channel);

                ctx.channel_update_callback(updated_channels);
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{channels, channel_service, channel_update_callback})
    };
}

CCoreCallbackData CallbackService::to_c_user_update_callback(Pubnub::User user_base, std::shared_ptr<const UserService> user_service, std::function<void (Pubnub::User)> user_update_callback) {
    struct CallbackCtx {
        Pubnub::User user_base;
        std::shared_ptr<const UserService> user_service;
        std::function<void (Pubnub::User)> user_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_user_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));
                auto user = ctx.user_service->create_user_object(Parsers::PubnubJson::to_user(message));
                auto updated = ctx.user_service->update_user_with_base(user, ctx.user_base);
                ctx.user_update_callback(updated);
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{user_base, user_service, user_update_callback})
    };
}

CCoreCallbackData CallbackService::to_c_users_updates_callback(const std::vector<Pubnub::User>& users, std::shared_ptr<const UserService> user_service, std::function<void(std::vector<Pubnub::User>)> user_update_callback) {
    struct CallbackCtx {
        const std::vector<Pubnub::User> users;
        std::shared_ptr<const UserService> user_service;
        std::function<void(std::vector<Pubnub::User>)> user_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_user_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));
                auto user = ctx.user_service->create_user_object(Parsers::PubnubJson::to_user(message));

                auto stream_user = std::find_if(
                        ctx.users.begin(),
                        ctx.users.end(),
                        [&user](const Pubnub::User& base_user) {
                            return base_user.user_id() == user.user_id();
                    });

                UserEntity stream_user_entity = UserDAO(stream_user->user_data()).to_entity();
                UserEntity user_entity = UserDAO(user.user_data()).to_entity();
                auto updated_user = ctx.user_service->create_user_object({stream_user->user_id(), UserEntity::from_base_and_updated_user(stream_user_entity, user_entity)});

                std::vector<Pubnub::User> updated_users;

                std::copy_if(
                        ctx.users.begin(),
                        ctx.users.end(),
                        std::back_inserter(updated_users), [&user](const Pubnub::User& base_user) {
                            return base_user.user_id() != user.user_id();
                    });

                updated_users.push_back(updated_user);

                ctx.user_update_callback(updated_users);
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{users, user_service, user_update_callback})
    };
}


CCoreCallbackData CallbackService::to_c_event_callback(Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(Pubnub::Event)> event_callback) {
    struct CallbackCtx {
        Pubnub::pubnub_chat_event_type chat_event_type;
        std::function<void(Pubnub::Event)> event_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_event(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));
                auto parsed_event = Parsers::PubnubJson::to_event(message);
                if (parsed_event.type == ctx.chat_event_type) {
                    ctx.event_callback(parsed_event);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{chat_event_type, event_callback})
    };
}

CCoreCallbackData CallbackService::to_c_presence_callback(Pubnub::String channel_id, std::shared_ptr<const PresenceService> presence_service, std::function<void(std::vector<Pubnub::String>)> presence_callback) {
    struct CallbackCtx {
        Pubnub::String channel_id;
        std::shared_ptr<const PresenceService> presence_service;
        std::function<void(std::vector<Pubnub::String>)> presence_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_presence(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                Pubnub::String normal_channel_name = Pubnub::String(message.channel.ptr, message.channel.size);
                auto pnpres_length = strlen("-pnpres");
                normal_channel_name.erase(message.channel.size - pnpres_length, pnpres_length);

                if (normal_channel_name == ctx.channel_id) {
                    ctx.presence_callback(ctx.presence_service->who_is_present(ctx.channel_id));
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{channel_id, presence_service, presence_callback})
    };
}


CCoreCallbackData CallbackService::to_c_membership_update_callback(Pubnub::Membership membership_base, std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Membership)> membership_callback) {
    struct CallbackCtx {
        Pubnub::Membership membership_base;
        std::weak_ptr<const ChatService> chat_service;
        std::function<void(Pubnub::Membership)> membership_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_membership_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto membership_channel = Parsers::PubnubJson::membership_channel(Pubnub::String(message.payload.ptr, message.payload.size));
                auto membership_user = Parsers::PubnubJson::membership_user(Pubnub::String(message.payload.ptr, message.payload.size));

                const auto channel_id = ctx.membership_base.channel.channel_id();
                const auto user_id = ctx.membership_base.user.user_id();

                if (membership_channel == channel_id && membership_user == user_id) {
                    auto custom_field = Parsers::PubnubJson::membership_from_string(Pubnub::String(message.payload.ptr, message.payload.size));
                    auto chat = ctx.chat_service.lock();
                    if (!chat) {
                        notify_error("Chat service is not available to call membership callback");
                        return;
                    }

                    Pubnub::Membership membership_obj = chat->membership_service->create_membership_object(
                            chat->user_service->get_user(user_id),
                            chat->channel_service->get_channel(channel_id),
                            custom_field
                    );
                    chat->membership_service->update_membership_with_base(membership_obj, ctx.membership_base);

                    ctx.membership_callback(membership_obj);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{membership_base, chat_service, membership_callback})
    };
}


CCoreCallbackData CallbackService::to_c_memberships_updates_callback(const std::vector<Pubnub::Membership>& memberships, std::weak_ptr<const ChatService> chat_service, std::function<void(std::vector<Pubnub::Membership>)> membership_callback) {
    struct CallbackCtx {
        const std::vector<Pubnub::Membership> memberships;
        std::weak_ptr<const ChatService> chat_service;
        std::function<void(std::vector<Pubnub::Membership>)> membership_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_membership_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));
    
                auto chat = ctx.chat_service.lock();
                if (!chat) {
                    notify_error("Chat service is not available to call membership callback");
                    return;
                }

                auto membership_channel = Parsers::PubnubJson::membership_channel(Pubnub::String(message.payload.ptr, message.payload.size));
                auto membership_user = Parsers::PubnubJson::membership_user(Pubnub::String(message.payload.ptr, message.payload.size));

                auto stream_membership = std::find_if(
                        ctx.memberships.begin(),
                        ctx.memberships.end(),
                        [&membership_channel, &membership_user](const Pubnub::Membership& base_membership) {
                            return base_membership.channel.channel_id() == membership_channel && base_membership.user.user_id() == membership_user;
                    });

                if (stream_membership != ctx.memberships.end()) {
                    auto custom_field = Parsers::PubnubJson::membership_from_string(Pubnub::String(message.payload.ptr, message.payload.size));
                    auto membership_obj = chat->membership_service->create_membership_object(
                            chat->user_service->get_user(membership_user),
                            chat->channel_service->get_channel(membership_channel),
                            custom_field
                    );

                    MembershipEntity stream_membership_entity = MembershipDAO(stream_membership->custom_data()).to_entity();
                    MembershipEntity membership_entity = MembershipDAO(membership_obj.custom_data()).to_entity();
                    auto updated_membership = chat->membership_service->create_membership_object(stream_membership->user, stream_membership->channel, MembershipEntity::from_base_and_updated_membership(stream_membership_entity, membership_entity));

                    std::vector<Pubnub::Membership> updated_memberships;

                    std::copy_if(
                            ctx.memberships.begin(),
                            ctx.memberships.end(),
                            std::back_inserter(updated_memberships),
                            [&membership_channel, &membership_user](const Pubnub::Membership& base_membership) {
                                return base_membership.channel.channel_id() != membership_channel || base_membership.user.user_id() != membership_user;
                        });

                    updated_memberships.push_back(updated_membership);

                    ctx.membership_callback(updated_memberships);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{memberships, chat_service, membership_callback})
    };
}


CCoreCallbackData CallbackService::to_c_message_update_callback(Pubnub::Message base_message, std::shared_ptr<const MessageService> message_service, std::function<void(Pubnub::Message)> message_update_callback) {
    struct CallbackCtx {
        Pubnub::Message base_message;
        std::shared_ptr<const MessageService> message_service;
        std::function<void(Pubnub::Message)> message_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_message_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto message_timetoken = Parsers::PubnubJson::message_update_timetoken(Pubnub::String(message.payload.ptr, message.payload.size));
                if (message_timetoken == ctx.base_message.timetoken()) {
                    auto parsed_message = ctx.message_service->create_message_object(Parsers::PubnubJson::to_message_update(message));
                    auto updated_message = ctx.message_service->update_message_with_base(parsed_message, ctx.base_message);

                    ctx.message_update_callback(updated_message);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{base_message, message_service, message_update_callback})
    };
}


CCoreCallbackData CallbackService::to_c_thread_message_update_callback(Pubnub::ThreadMessage base_message, std::shared_ptr<const MessageService> message_service, std::function<void(Pubnub::ThreadMessage)> message_update_callback) {
    struct CallbackCtx {
        Pubnub::ThreadMessage base_message;
        std::shared_ptr<const MessageService> message_service;
        std::function<void(Pubnub::ThreadMessage)> message_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_message_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto message_timetoken = Parsers::PubnubJson::message_update_timetoken(Pubnub::String(message.payload.ptr, message.payload.size));
                if (message_timetoken == ctx.base_message.timetoken()) {
                    auto parsed_message = ctx.message_service->create_message_object(Parsers::PubnubJson::to_message_update(message));
                    auto updated_message = ctx.message_service->update_message_with_base(parsed_message, ctx.base_message);

                    ctx.message_update_callback(Pubnub::ThreadMessage(updated_message, ctx.base_message.parent_channel_id()));
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{base_message, message_service, message_update_callback})
    };
}

CCoreCallbackData CallbackService::to_c_messages_updates_callback(const std::vector<Pubnub::Message>& messages, std::shared_ptr<const MessageService> message_service, std::function<void(std::vector<Pubnub::Message>)> message_update_callback) {
    struct CallbackCtx {
        std::vector<Pubnub::Message> messages;
        std::shared_ptr<const MessageService> message_service;
        std::function<void(std::vector<Pubnub::Message>)> message_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message pn_message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_message_update(Pubnub::String(pn_message.payload.ptr, pn_message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto message_timetoken = Parsers::PubnubJson::message_update_timetoken(Pubnub::String(pn_message.payload.ptr, pn_message.payload.size));
                auto message = std::find_if(
                        ctx.messages.begin(),
                        ctx.messages.end(),
                        [&message_timetoken](const Pubnub::Message& base_message) {
                            return base_message.timetoken() == message_timetoken;
                    });

                if (message != ctx.messages.end()) {
                    auto parsed_message = ctx.message_service->create_message_object(Parsers::PubnubJson::to_message_update(pn_message));
                    auto updated_message = ctx.message_service->update_message_with_base(parsed_message, *message);

                    std::vector<Pubnub::Message> updated_messages;

                    std::copy_if(
                            ctx.messages.begin(),
                            ctx.messages.end(),
                            std::back_inserter(updated_messages), [&message_timetoken](const Pubnub::Message& base_message) {
                                return base_message.timetoken() != message_timetoken;
                        });

                    updated_messages.push_back(updated_message);

                    ctx.message_update_callback(updated_messages);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{messages, message_service, message_update_callback})
    };
}


CCoreCallbackData CallbackService::to_c_thread_messages_updates_callback(const std::vector<Pubnub::ThreadMessage>& messages, std::shared_ptr<const MessageService> message_service, std::function<void(std::vector<Pubnub::ThreadMessage>)> message_update_callback) {
    struct CallbackCtx {
        const std::vector<Pubnub::ThreadMessage> messages;
        std::shared_ptr<const MessageService> message_service;
        std::function<void(std::vector<Pubnub::ThreadMessage>)> message_update_callback;
    };

    return {
        +[](const pubnub_t* pb, struct pubnub_v2_message pn_message, void* ctx_ptr) {
            if (Parsers::PubnubJson::is_message_update(Pubnub::String(pn_message.payload.ptr, pn_message.payload.size))) {
                auto ctx = std::any_cast<CallbackCtx>(*reinterpret_cast<std::any*>(ctx_ptr));

                auto message_timetoken = Parsers::PubnubJson::message_update_timetoken(Pubnub::String(pn_message.payload.ptr, pn_message.payload.size));
                auto message = std::find_if(
                        ctx.messages.begin(),
                        ctx.messages.end(),
                        [&message_timetoken](const Pubnub::ThreadMessage& base_message) {
                            return base_message.timetoken() == message_timetoken;
                    });

                if (message != ctx.messages.end()) {
                    auto parsed_message = ctx.message_service->create_message_object(Parsers::PubnubJson::to_message_update(pn_message));
                    auto updated_message = ctx.message_service->update_message_with_base(parsed_message, *message);

                    std::vector<Pubnub::ThreadMessage> updated_messages;

                    std::copy_if(
                            ctx.messages.begin(),
                            ctx.messages.end(),
                            std::back_inserter(updated_messages),
                            [&message_timetoken](const Pubnub::ThreadMessage& base_message) {
                                return base_message.timetoken() != message_timetoken;
                        });

                    updated_messages.push_back(Pubnub::ThreadMessage(updated_message, message->parent_channel_id()));

                    ctx.message_update_callback(updated_messages);
                }
            }
        },
        this->callback_contexts.emplace_back(CallbackCtx{messages, message_service, message_update_callback})
    };
}
