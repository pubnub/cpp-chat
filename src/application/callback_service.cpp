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

// Code below is inspired by this post and adjusted to our needs:
// https://stackoverflow.com/a/73177293/11933889
template <class Lambda, class... Args> static auto make_c_callable(Lambda l) {
    static Lambda callback = std::move(l);
    return +[](Args... args) { return callback(args...); };
}

template <class Lambda>
struct CConverter : public CConverter<decltype(&Lambda::operator())> {};

template <class CCoreCallback, class... Args>
struct CConverter<void(CCoreCallback::*)(Args...) const> {
  template <class Lambda> static auto to_c_function(Lambda l) {
    return make_c_callable<Lambda, Args...>(std::move(l));
  }
};

template <class CCoreCallback, class... Args>
struct CConverter<void(CCoreCallback::*)(Args...)> {
  template <class Lambda> static auto to_c_function(Lambda l) {
    return make_c_callable<Lambda, Args...>(std::move(l));
  }
};

template <class Lambda> auto to_c_core_callback(Lambda lambda) { return CConverter<Lambda>::to_c_function(std::move(lambda)); }

static void notify_error(const Pubnub::String& error_message) {
    // Basically, we cannot throw exceptions in C callbacks, so we just print the error message 
    // and let the program continue because we cannot do anything else.
    // TODO: Logger
    std::cout << "Callback error: " << error_message << std::endl;
}

pubnub_subscribe_message_callback_t CallbackService::to_c_message_callback(std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Message)> message_callback) {
    return to_c_core_callback([chat_service, message_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_message(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto parsed_message = Parsers::PubnubJson::to_message(message);
                if (auto service = chat_service.lock()) {
                    message_callback(service->message_service->create_message_object(parsed_message));
                } else {
                    notify_error("Chat service is not available to call message callback");
                }
                }
            });
}

pubnub_subscribe_message_callback_t CallbackService::to_c_channel_update_callback(Pubnub::Channel base_channel, std::shared_ptr<const ChannelService> channel_service, std::function<void (Pubnub::Channel)> channel_update_callback) {
    return to_c_core_callback([base_channel, channel_service, channel_update_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_channel_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto channel = channel_service->create_channel_object(Parsers::PubnubJson::to_channel(message));
                auto updated = channel_service->update_channel_with_base(channel, base_channel);
                channel_update_callback(updated);
            }
        });
}


pubnub_subscribe_message_callback_t CallbackService::to_c_channels_updates_callback(const std::vector<Pubnub::Channel>& channels, std::shared_ptr<const ChannelService> channel_service, std::function<void(std::vector<Pubnub::Channel>)> channel_update_callback) {
    return to_c_core_callback([channels, channel_service, channel_update_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_channel_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto channel = channel_service->create_channel_object(Parsers::PubnubJson::to_channel(message));

                auto stream_channel = std::find_if(channels.begin(), channels.end(), [&channel](const Pubnub::Channel& base_channel) {
                        return base_channel.channel_id() == channel.channel_id();
                });

                ChannelEntity stream_channel_entity = ChannelDAO(stream_channel->channel_data()).to_entity();
                ChannelEntity channel_entity = ChannelDAO(channel.channel_data()).to_entity();
                auto pair = std::make_pair(channel.channel_id(), ChannelEntity::from_base_and_updated_channel(stream_channel_entity, channel_entity));
                auto updated_channel = channel_service->create_channel_object(pair);

                std::vector<Pubnub::Channel> updated_channels;

                std::copy_if(channels.begin(), channels.end(), std::back_inserter(updated_channels), [&channel](const Pubnub::Channel& base_channel) {
                        return base_channel.channel_id() != channel.channel_id();
                });

                updated_channels.push_back(updated_channel);

                channel_update_callback(updated_channels);
            }
        });
}

pubnub_subscribe_message_callback_t CallbackService::to_c_user_update_callback(Pubnub::User user_base, std::shared_ptr<const UserService> user_service, std::function<void (Pubnub::User)> user_update_callback) {
    return to_c_core_callback([user_base, user_service, user_update_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_user_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto user = user_service->create_user_object(Parsers::PubnubJson::to_user(message));
                auto updated = user_service->update_user_with_base(user, user_base);
                user_update_callback(updated);
            }
        });
}


pubnub_subscribe_message_callback_t CallbackService::to_c_event_callback(Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(Pubnub::Event)> event_callback) {
    return to_c_core_callback([chat_event_type, event_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_event(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto parsed_event = Parsers::PubnubJson::to_event(message);
                if (parsed_event.type == chat_event_type) {
                    event_callback(parsed_event);
                }
            }
        });
}

pubnub_subscribe_message_callback_t CallbackService::to_c_presence_callback(Pubnub::String channel_id, std::shared_ptr<const PresenceService> presence_service, std::function<void(std::vector<Pubnub::String>)> presence_callback) {
    return to_c_core_callback([channel_id, presence_service, presence_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_presence(Pubnub::String(message.payload.ptr, message.payload.size))) {
                Pubnub::String normal_channel_name = Pubnub::String(message.channel.ptr, message.channel.size);
                auto pnpres_length = strlen("-pnpres");
                normal_channel_name.erase(message.channel.size - pnpres_length, pnpres_length);

                if (normal_channel_name == channel_id) {
                    presence_callback(presence_service->who_is_present(channel_id));
                }
            }
        });
}


pubnub_subscribe_message_callback_t CallbackService::to_c_membership_update_callback(Pubnub::Membership membership_base, std::weak_ptr<const ChatService> chat_service, std::function<void(Pubnub::Membership)> membership_callback) {
    return to_c_core_callback([membership_base, chat_service, membership_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            if (Parsers::PubnubJson::is_membership_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto membership_channel = Parsers::PubnubJson::membership_channel(Pubnub::String(message.payload.ptr, message.payload.size));
                auto membership_user = Parsers::PubnubJson::membership_user(Pubnub::String(message.payload.ptr, message.payload.size));

                const auto channel_id = membership_base.channel.channel_id();
                const auto user_id = membership_base.user.user_id();

                if (membership_channel == channel_id && membership_user == user_id) {
                    auto custom_field = Parsers::PubnubJson::membership_from_string(Pubnub::String(message.payload.ptr, message.payload.size));
                    auto chat = chat_service.lock();
                    if (!chat) {
                        notify_error("Chat service is not available to call membership callback");
                        return;
                    }

                    Pubnub::Membership membership_obj = chat->membership_service->create_membership_object(
                            chat->user_service->get_user(user_id),
                            chat->channel_service->get_channel(channel_id),
                            custom_field
                    );
                    chat->membership_service->update_membership_with_base(membership_obj, membership_base);

                    membership_callback(membership_obj);
                }
            }
        });
}


pubnub_subscribe_message_callback_t CallbackService::to_c_memberships_updates_callback(const std::vector<Pubnub::Membership>& memberships, std::weak_ptr<const ChatService> chat_service, std::function<void(std::vector<Pubnub::Membership>)> membership_callback) {
    return to_c_core_callback([memberships, chat_service, membership_callback](const pubnub_t* pb, struct pubnub_v2_message message) {
            auto chat = chat_service.lock();
            if (!chat) {
                notify_error("Chat service is not available to call membership callback");
                return;
            }

            if (Parsers::PubnubJson::is_membership_update(Pubnub::String(message.payload.ptr, message.payload.size))) {
                auto membership_channel = Parsers::PubnubJson::membership_channel(Pubnub::String(message.payload.ptr, message.payload.size));
                auto membership_user = Parsers::PubnubJson::membership_user(Pubnub::String(message.payload.ptr, message.payload.size));

                auto stream_membership = std::find_if(memberships.begin(), memberships.end(), [&membership_channel, &membership_user](const Pubnub::Membership& base_membership) {
                        return base_membership.channel.channel_id() == membership_channel && base_membership.user.user_id() == membership_user;
                });

                if (stream_membership != memberships.end()) {
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

                    std::copy_if(memberships.begin(), memberships.end(), std::back_inserter(updated_memberships), [&membership_channel, &membership_user](const Pubnub::Membership& base_membership) {
                            return base_membership.channel.channel_id() != membership_channel || base_membership.user.user_id() != membership_user;
                    });

                    updated_memberships.push_back(updated_membership);

                    membership_callback(updated_memberships);
                }
            }
        });
}
