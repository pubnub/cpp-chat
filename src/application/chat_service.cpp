#include "chat_service.hpp"
#include "application/bundles.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/message_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/membership_service.hpp"
#include "application/callback_service.hpp"
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub)
{}

void ChatService::init_services() {
    channel_service = std::make_shared<ChannelService>(pubnub, weak_from_this());
    user_service = std::make_shared<UserService>(pubnub, weak_from_this());
    message_service = std::make_shared<MessageService>(pubnub, weak_from_this());
    membership_service = std::make_shared<MembershipService>(pubnub, weak_from_this());
    presence_service = std::make_shared<PresenceService>(pubnub, weak_from_this());
    restrictions_service = std::make_shared<RestrictionsService>(pubnub, weak_from_this());
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

ThreadSafePtr<PubNub> ChatService::create_pubnub(const String& publish_key, const String& subscribe_key, const String& user_id) {
    return std::make_shared<Mutex<PubNub>>(publish_key, subscribe_key, user_id);
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

void ChatService::listen_for_events(const Pubnub::String& channel_id, Pubnub::pubnub_chat_event_type chat_event_type, std::function<void(const Pubnub::String&)> event_callback) const {
    if(channel_id.empty())
    {
        throw std::invalid_argument("Cannot listen for events - channel_id is empty");
    }

    auto messages = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->subscribe_to_channel_and_get_messages(channel_id);
    }();

    // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
    // First broadcast messages because they're not related to the new callback
    this->callback_service->broadcast_messages(messages);
    this->callback_service->register_event_callback(channel_id, chat_event_type, event_callback);
#endif
}
