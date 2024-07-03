#include "presentation/chat.hpp"
#include "application/chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"

using namespace Pubnub;

Chat::Chat(String publish_key, String subscribe_key, String user_id) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(publish_key, subscribe_key, user_id)
            )
        ),
    channel_service(chat_service->channel_service),
    user_service(chat_service->user_service)
{}

Channel Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}

Channel Chat::get_channel(String channel_id)
{
    return this->channel_service->get_channel(channel_id);
}

std::vector<Channel> Chat::get_channels(String include, int limit, String start, String end)
{
    return this->channel_service->get_channels(include, limit, start, end);
}

Channel Chat::update_channel(String channel_id, ChatChannelData channel_data)
{
    return this->channel_service->update_channel(channel_id, channel_data);
}

void Chat::delete_channel(String channel_id)
{
    this->channel_service->delete_channel(channel_id);
}

void Chat::pin_message_to_channel(Pubnub::Message message, Pubnub::Channel channel)
{
    this->channel_service->pin_message_to_channel(message, channel);
}

void Chat::unpin_message_from_channel(Pubnub::Channel channel)
{
    this->channel_service->unpin_message_from_channel(channel);
}

User Chat::create_user(String user_id, ChatUserData user_data)
{
    return user_service->create_user(user_id, user_data);
}

User Chat::get_user(String user_id)
{
    return user_service->get_user(user_id);
}

std::vector<User> Chat::get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end)
{
    return user_service->get_users(include, limit, start, end);
}

User Chat::update_user(String user_id, ChatUserData user_data)
{
    return user_service->update_user(user_id, user_data);
}

void Chat::delete_user(String user_id)
{
    return user_service->delete_user(user_id);
}

std::vector<Pubnub::String> Chat::where_present(Pubnub::String user_id)
{
    return this->user_service->where_present(user_id);
}

std::vector<Pubnub::String> Chat::who_is_present(Pubnub::String channel_id)
{
    return this->channel_service->who_is_present(channel_id);
}

bool Chat::is_present(Pubnub::String user_id, Pubnub::String channel_id)
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