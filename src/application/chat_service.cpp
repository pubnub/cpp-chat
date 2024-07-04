#include "chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/message_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub),
channel_service(std::make_shared<ChannelService>(pubnub, entity_repository, weak_from_this())),
user_service(std::make_shared<UserService>(pubnub, entity_repository, weak_from_this())),
message_service(std::make_shared<MessageService>(pubnub, entity_repository, weak_from_this())),
presence_service(std::make_shared<PresenceService>(pubnub, entity_repository, weak_from_this())),
restrictions_service(std::make_shared<RestrictionsService>(pubnub, entity_repository, weak_from_this()))
{}

ThreadSafePtr<PubNub> ChatService::create_pubnub(String publish_key, String subscribe_key, String user_id)
{
    return std::make_shared<Mutex<PubNub>>(publish_key, subscribe_key, user_id);
}

void ChatService::emit_chat_event(pubnub_chat_event_type chat_event_type, String channel_id, String payload)
{
    //Payload is in form of Json: {"param1": "param1value", "param2": "param2value" ... }. So in order to get just parameters, we remove first and last curl bracket
	String payload_parameters = payload;
    payload_parameters.erase(0, 1);
	payload_parameters.erase(payload_parameters.length() - 1);
	String event_message = String("{") + payload_parameters + String(", \"type\": \"") + chat_event_type_to_string(chat_event_type) + String("\"}");

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->publish(channel_id, event_message);
}