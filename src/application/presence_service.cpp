#include "presence_service.hpp"
#include "domain/presence.hpp"
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

std::vector<Pubnub::String> PresenceService::who_is_present(const Pubnub::String& channel_id) const {
    auto here_now_response = [this, channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->here_now(channel_id);
    }();

    json response_json = json::parse(here_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get who is present, response is incorrect");
    }

    if (response_json["uuids"].is_null())
    {
        return {};
    }
    
    return Presence::users_from_response(response_json);
}

std::vector<String> PresenceService::where_present(const String& user_id) const {
    auto where_now_response = [this, user_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->where_now(user_id);
    }();

    json response_json = json::parse(where_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get where present, response is incorrect");
    }

    if(response_json["payload"].is_null())
    {
        throw std::runtime_error("can't get where present, payload is incorrect");
    }

    if(response_json["payload"]["channels"].is_null())
    {
        return {};
    }

    return Presence::channels_from_response(response_json);
}

bool PresenceService::is_present(const Pubnub::String& user_id, const Pubnub::String& channel_id) const {
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

void PresenceService::stream_presence(const Pubnub::String& channel_id, std::function<void(const std::vector<Pubnub::String>&)> presence_callback)
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
