#include "chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/message_service.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

ChatService::ChatService(ThreadSafePtr<PubNub> pubnub):
pubnub(pubnub),
channel_service(std::make_shared<ChannelService>(pubnub, entity_repository, weak_from_this())),
user_service(std::make_shared<UserService>(pubnub, entity_repository, weak_from_this())),
message_service(std::make_shared<MessageService>(pubnub, entity_repository, weak_from_this()))
{}

ThreadSafePtr<PubNub> ChatService::create_pubnub(Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id)
{
    return std::make_shared<Mutex<PubNub>>(publish_key, subscribe_key, user_id);
}