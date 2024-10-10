#include "chat_service.hpp"
#include "application/bundles.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/message_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/membership_service.hpp"
#include "application/callback_service.hpp"
#include "application/access_manager_service.hpp"
#include "chat.hpp"
#include "domain/parsers.hpp"
#include "domain/json.hpp"
#include "domain/timetoken.hpp"
#include "enums.hpp"
#include "infra/pubnub.hpp"
#include "infra/rate_limiter.hpp"
#include "mentions.hpp"
#include "nlohmann/json.hpp"
#include "domain/parsers.hpp"
#include <pubnub_helper.h>
#include "mentions.hpp"

#ifdef PN_CHAT_C_ABI
extern "C" {
    #include <pubnub_subscribe_v2.h>
}
#endif

using namespace Pubnub;
using json = nlohmann::json;

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub)
{}

void ChatService::init_services(const ChatConfig& config) {
    channel_service = std::make_shared<ChannelService>(pubnub, weak_from_this(), config.rate_limit_factor);
    user_service = std::make_shared<UserService>(pubnub, weak_from_this(), config.store_user_activity_interval);
    message_service = std::make_shared<MessageService>(pubnub, weak_from_this());
    membership_service = std::make_shared<MembershipService>(pubnub, weak_from_this());
    presence_service = std::make_shared<PresenceService>(pubnub, weak_from_this());
    restrictions_service = std::make_shared<RestrictionsService>(pubnub, weak_from_this());
    access_manager_service = std::make_shared<AccessManagerService>(pubnub, config.auth_key);
    this->chat_config = config;
#ifndef PN_CHAT_C_ABI
    auto service_bundle = EntityServicesBundle{
        channel_service,
        user_service,
        weak_from_this(),
        message_service,
        membership_service,
    };
    this->callback_service = std::make_shared<CallbackService>(
        service_bundle,
        presence_service,
        pubnub
    );
#endif
}

ThreadSafePtr<PubNub> ChatService::create_pubnub(const String& publish_key, const String& subscribe_key, const String& user_id, const String& auth_key) {
    return std::make_shared<Mutex<PubNub>>(publish_key, subscribe_key, user_id, auth_key);
}

void ChatService::emit_chat_event(pubnub_chat_event_type chat_event_type, const String& channel_id, const String& payload, EventMethod event_method) const {
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + chat_event_type_to_string(chat_event_type) + String("\"}");

    auto pubnub_handle = this->pubnub->lock();
    if (event_method == EventMethod::Default)
    {
        event_method = event_method_from_event_type(chat_event_type);
    }

    event_method == EventMethod::Signal 
        ? pubnub_handle->signal(channel_id, event_message) 
        : pubnub_handle->publish(channel_id, event_message);
}

std::tuple<std::vector<Pubnub::Event>, bool> ChatService::get_events_history(const Pubnub::String &channel_id, const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count) const
{
    auto fetch_history_response = [this, channel_id, start_timetoken, end_timetoken, count] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->fetch_history(channel_id, start_timetoken, end_timetoken, count);
    }();

    Json response_json = Json::parse(fetch_history_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get history, response is incorrect");
    }

    if(!response_json.contains("channels") && !response_json["channels"].contains(channel_id))
    {
        throw std::runtime_error("can't get events history, response doesn't have this channel info");
    }

    Json messages_array_json = response_json["channels"];

    std::vector<Event> events;
    bool is_more = count == messages_array_json.size();

    if (!messages_array_json.contains(channel_id))
    {
        std::tuple<std::vector<Pubnub::Event>, bool> return_tuple = std::make_tuple(events, is_more);
        return return_tuple;
    }
    for (auto element : messages_array_json[channel_id])
    {
        if(!element.contains("message") || element["message"].is_null())
        {
            continue;
        }

        Json message_json = element["message"];

        if(!Parsers::PubnubJson::is_event(message_json.dump()))
        {
            continue;
        }
        
        Event event;
        event.channel_id = channel_id;
        event.timetoken = element.get_string("timetoken").value_or(String(""));
        event.user_id = element.get_string("uuid").value_or(String(""));
        event.type = chat_event_type_from_string(message_json.get_string("type").value_or(String("")));
        event.payload = message_json.dump();
        events.push_back(event);
    }

    std::tuple<std::vector<Pubnub::Event>, bool> return_tuple = std::make_tuple(events, is_more);
    return return_tuple;
}

std::tuple<std::vector<Pubnub::UserMentionData>, bool> ChatService::get_current_user_mentions(const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const
{
    // TODO: overcomplicated with this user id
    auto current_user = this->pubnub->lock()->get_user_id();
    auto events_history = this->get_events_history(current_user, start_timetoken, end_timetoken, count);

    auto events = std::get<0>(events_history);

    events.erase(
            std::remove_if(
                events.begin(),
                events.end(),
                [current_user](const Event& event) {
                    return event.type != pubnub_chat_event_type::PCET_MENTION;
                }),
            events.end()
    );

    std::vector<Pubnub::UserMentionData> enchanced_events;

    std::transform(
            events.begin(),
            events.end(),
            std::back_inserter(enchanced_events),
            [this](const Event& event) {
                auto payload_json = Json::parse(event.payload);

                auto timetoken = payload_json.get_string("messageTimetoken");
                auto channel_id = payload_json.get_string("channel");

                if(!timetoken.has_value())
                {
                    throw std::runtime_error("can't get message timetoken from payload");
                }

                if(!channel_id.has_value())
                {
                    throw std::runtime_error("can't get channel id from payload");
                }

                auto previous_timetoken = Timetoken::increase_by_one(timetoken.value());

                auto messages = [this, previous_timetoken, timetoken, channel_id] {
                    auto pubnub_handle = this->pubnub->lock();
                    return pubnub_handle->fetch_history(channel_id.value(), previous_timetoken, timetoken.value(), 1);
                }();

                auto messages_json = Json::parse(messages);

                auto message = this->message_service->create_message_object(
                        MessageEntity::from_history_json(messages_json, channel_id.value())[0]);

                if (!messages_json.contains("parentChannel)")) {
                    return UserMentionData{
                        channel_id.value(),
                        event.user_id,
                        event,
                        message,
                        Pubnub::Option<Pubnub::String>::none(),
                        Pubnub::Option<Pubnub::String>::none()
                    };
                }

                return UserMentionData{
                    channel_id.value(),
                    event.user_id,
                    event,
                    message,
                    messages_json.get_string("parentChannel"),
                    messages_json.get_string("threadChannel")
                };
            }
    );

    bool is_more = std::get<1>(events_history);

    return std::make_tuple(enchanced_events, is_more);
}

#ifndef PN_CHAT_C_ABI
std::function<void()> ChatService::listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(const Pubnub::Event&)> event_callback) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Cannot listen for events - channel_id is empty");
    }

    auto messages = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->subscribe_to_channel_and_get_messages(channel_id);
    }();

    // First broadcast messages because they're not related to the new callback
    this->callback_service->broadcast_messages(messages);
    this->callback_service->register_event_callback(channel_id, chat_event_type, event_callback);

    std::function<void()> stop_callback = [=](){
        this->callback_service->remove_event_callback(channel_id, chat_event_type);
    };
    return stop_callback;
}

#else

std::vector<pubnub_v2_message> ChatService::listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Cannot listen for events - channel_id is empty");
    }

    auto messages = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->subscribe_to_channel_and_get_messages(channel_id);
    }();
    
    return messages;
}

std::vector<pubnub_v2_message> ChatService::get_chat_updates() const
{
    auto messages = [this] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->fetch_messages();
    }();

    return messages;
};

#endif
