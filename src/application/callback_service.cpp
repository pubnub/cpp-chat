#include "callback_service.hpp"
#include "application/bundles.hpp"
#include "enums.hpp"
#include "infra/serialization.hpp"
#include "infra/sync.hpp"
#include "nlohmann/json.hpp"
#include "domain/parsers.hpp"
#include "presentation/message.hpp"

using json = nlohmann::json;

CallbackService::CallbackService(
        EntityServicesBundle entity_bundle,
        std::weak_ptr<PresenceService> presence_service,
        ThreadSafePtr<PubNub> pubnub,
        milliseconds wait_interval) :
    pubnub(pubnub),
    thread_run_flag(true),
    chat_service(entity_bundle.chat_service),
    message_service(entity_bundle.message_service),
    channel_service(entity_bundle.channel_service),
    user_service(entity_bundle.user_service),
    membership_service(entity_bundle.membership_service),
    presence_service(presence_service)
{
    this->callback_thread = std::thread([this, wait_interval]() {
        while (this->thread_run_flag.load()) {
            this->resolve_callbacks();
            // TODO: should we sleep in tests?
            std::this_thread::sleep_for(wait_interval);
        }
    });
}

void CallbackService::resolve_callbacks() {
    auto messages = [this]{
        auto guard = this->pubnub->lock();
        return guard->fetch_messages();
    }();

    for (auto message : messages) {
    }
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
            auto parsed_event = Parsers::PubnubJson::message_string(message);

            Pubnub::pubnub_chat_event_type event_type;
            std::function<void(Pubnub::String)> callback;
            std::tie(event_type, callback) = maybe_callback.value();

            auto message_event_type = Parsers::PubnubJson::event_type(parsed_event);
            if (Pubnub::chat_event_type_from_string(message_event_type) == event_type) {
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
        
        auto maybe_callback = this->callbacks.get_message_update_callbacks().get(message_timetoken);
        if (maybe_callback.has_value()) {
            Pubnub::String message_channel;
            std::function<void(Pubnub::Message)> callback;
            std::tie(message_channel, callback) = maybe_callback.value();
            if (auto service = this->channel_service.lock()) {
                callback(service->get_channel(message_channel).get_message(message_timetoken));
            } else {
                throw std::runtime_error("Channel service is not available to call callback");
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
                auto custom_field = Parsers::PubnubJson::to_membership(message_string);
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


