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
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"
#include "domain/parsers.hpp"
#include <pubnub_helper.h>

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
    channel_service = std::make_shared<ChannelService>(pubnub, weak_from_this());
    user_service = std::make_shared<UserService>(pubnub, weak_from_this());
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

void ChatService::emit_chat_event(pubnub_chat_event_type chat_event_type, const String& channel_id, const String& payload) const {
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + chat_event_type_to_string(chat_event_type) + String("\"}");

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->publish(channel_id, event_message);
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
    bool is_more = count == messages_array_json.size();
    std::tuple<std::vector<Pubnub::Event>, bool> return_tuple = std::make_tuple(events, is_more);
    return return_tuple;
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
