#include "presentation/chat.hpp"
#include "application/chat_service.hpp"
#include "application/channel_service.hpp"

using namespace Pubnub;

Chat::Chat(String publish_key, String subscribe_key, String user_id) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(publish_key, subscribe_key, user_id)
            )
        ),
    channel_service(chat_service->channel_service)
{}

Channel Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}
