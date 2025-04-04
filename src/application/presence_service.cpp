#include "presence_service.hpp"
#include <memory>
#include "domain/presence.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"
#include "string.hpp"

using namespace Pubnub;
using json = nlohmann::json;

PresenceService::PresenceService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
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

std::shared_ptr<Subscription> PresenceService::stream_presence(const Pubnub::String& channel_id, std::function<void(const std::vector<Pubnub::String>&)> presence_callback) const
{
    //Send callback with currently present users
    std::vector<Pubnub::String> current_users = who_is_present(channel_id);
    presence_callback(current_users);

    auto pubnub_handle = this->pubnub->lock();

    String presence_channel = channel_id + "-pnpres";
    
    auto subscription = this->pubnub->lock()->subscribe(presence_channel);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_presence_listener(callback_service->to_c_presence_callback(presence_channel, this->shared_from_this(), presence_callback));

    return subscription;
}

