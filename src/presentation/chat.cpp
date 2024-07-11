#include "presentation/chat.hpp"
#include "application/chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"
#include "application/membership_service.hpp"

using namespace Pubnub;

Chat::Chat(String publish_key, String subscribe_key, String user_id) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(publish_key, subscribe_key, user_id),
                ChatService::create_entity_repository()
            )
        )
{
    chat_service->init_services();
    channel_service = chat_service->channel_service;
    user_service = chat_service->user_service;
    presence_service = chat_service->presence_service;
    restrictions_service = chat_service->restrictions_service;
    message_service = chat_service->message_service;
    membership_service = chat_service->membership_service;
#ifndef PN_CHAT_C_ABI
    callback_service = chat_service->callback_service;
#endif
}

Channel Chat::create_public_conversation(String channel_id, ChatChannelData channel_data)
{
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}

CreatedChannelWrapper Chat::create_direct_conversation(User user, String channel_id, ChatChannelData channel_data, String membership_data)
{
    auto return_tuple = this->channel_service->create_direct_conversation(user, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), std::get<2>(return_tuple));
}

CreatedChannelWrapper Chat::create_group_conversation(std::vector<User> users, String channel_id, ChatChannelData channel_data, String membership_data)
{
    auto return_tuple = this->channel_service->create_group_conversation(users, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), std::get<2>(return_tuple));
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

void Chat::pin_message_to_channel(Message message, Channel channel)
{
    this->channel_service->pin_message_to_channel(message, channel);
}

void Chat::unpin_message_from_channel(Channel channel)
{
    this->channel_service->unpin_message_from_channel(channel);
}

User Chat::create_user(String user_id, ChatUserData user_data)
{
    return this->user_service->create_user(user_id, user_data);
}

User Chat::get_user(String user_id)
{
    return this->user_service->get_user(user_id);
}

std::vector<User> Chat::get_users(String include, int limit, String start, String end)
{
    return this->user_service->get_users(include, limit, start, end);
}

User Chat::update_user(String user_id, ChatUserData user_data)
{
    return this->user_service->update_user(user_id, user_data);
}

void Chat::delete_user(String user_id)
{
    return this->user_service->delete_user(user_id);
}

std::vector<String> Chat::where_present(String user_id)
{
    return this->presence_service->where_present(user_id);
}

std::vector<String> Chat::who_is_present(String channel_id)
{
    return this->presence_service->who_is_present(channel_id);
}

bool Chat::is_present(String user_id, String channel_id)
{
    return this->presence_service->is_present(user_id, channel_id);
}

void Chat::set_restrictions(String user_id, String channel_id, Restriction restrictions)
{
    this->restrictions_service->set_restrictions(user_id, channel_id, restrictions);
}

void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, String channel_id, String payload)
{
    this->chat_service->emit_chat_event(chat_event_type, channel_id, payload);
}

void Chat::listen_for_events(String channel_id, pubnub_chat_event_type chat_event_type, std::function<void(String)> event_callback)
{
    this->chat_service->listen_for_events(channel_id, chat_event_type, event_callback);
}

void Chat::forward_message(Message message, Channel channel)
{
    this->message_service->forward_message(message, channel.channel_id());
}

std::vector<UnreadMessageWrapper> Chat::get_unread_message_counts(String start_timetoken, String end_timetoken, String filter, int limit)
{
    auto tuples = this->membership_service->get_all_unread_messages_counts(start_timetoken, end_timetoken, filter, limit);

    std::vector<UnreadMessageWrapper> return_wrappers;
    for(auto &tuple : tuples)
    {
        UnreadMessageWrapper wrapper = {std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)};
        return_wrappers.push_back(wrapper);
    }

    return return_wrappers;
}
