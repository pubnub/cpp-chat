#include "chat_service.hpp"
#include "application/channel_service.hpp"

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub),
channel_service(std::make_shared<ChannelService>(ChannelService(pubnub, entity_repository)))
{}