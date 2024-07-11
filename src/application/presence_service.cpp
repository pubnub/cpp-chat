#include "presence_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"

using namespace Pubnub;
using json = nlohmann::json;

PresenceService::PresenceService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

std::vector<Pubnub::String> PresenceService::who_is_present(Pubnub::String channel_id)
{
    auto pubnub_handle = this->pubnub->lock();
    String here_now_response = pubnub_handle->here_now(channel_id);

    json response_json = json::parse(here_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get who is present, response is incorrect");
    }

    json uuids_array_json = response_json["uuids"];

    std::vector<String> user_ids;
   
    for (json::iterator it = uuids_array_json.begin(); it != uuids_array_json.end(); ++it) 
    {
        user_ids.push_back(static_cast<String>(*it));
    }
    
    return user_ids;
}

std::vector<String> PresenceService::where_present(String user_id)
{
    auto pubnub_handle = this->pubnub->lock();
    String where_now_response = pubnub_handle->where_now(user_id);

    json response_json = json::parse(where_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get where present, response is incorrect");
    }

    json response_payload_json = response_json["payload"];
    json channels_array_json = response_payload_json["channels"];

    std::vector<String> channel_ids;
   
    for (json::iterator it = channels_array_json.begin(); it != channels_array_json.end(); ++it) 
    {
        channel_ids.push_back(static_cast<String>(*it));
    }
    
    return channel_ids;
}

bool PresenceService::is_present(Pubnub::String user_id, Pubnub::String channel_id)
{
    std::vector<String> channels = this->where_present(user_id);
    //TODO: we should us std::count here, but it didn't work
    int count = 0;
    for( auto channel : channels)
    {
        if(channel_id == channel)
        {
            count = 1;
            break;
        }
    }
    //int count = std::count(channels.begin(), channels.end(), channel_id);
    return count > 0;
}

void PresenceService::stream_presence(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> presence_callback)
{
    //Send callback with currently present users
    std::vector<Pubnub::String> current_users = who_is_present(channel_id);
    presence_callback(current_users);

    auto pubnub_handle = this->pubnub->lock();

    String presence_channel = channel_id + "-pnpres";
    auto messages = pubnub_handle->subscribe_to_channel_and_get_messages(presence_channel);

    // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock())
    {
        chat->callback_service->broadcast_messages(messages);
        chat->callback_service->register_channel_presence_callback(channel_id, presence_callback);
    }
#endif
}
