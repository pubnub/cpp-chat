#include "chat_service.hpp"
#include "application/channel_service.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub),
channel_service(std::make_shared<ChannelService>(pubnub, entity_repository))
{}

ThreadSafePtr<PubNub> ChatService::create_pubnub(Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id)
{
    return std::make_shared<Mutex<PubNub>>(publish_key, subscribe_key, user_id);
}