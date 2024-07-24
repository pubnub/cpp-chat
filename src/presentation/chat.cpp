#include "chat.hpp"
#include "application/chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"
#include "application/membership_service.hpp"
#include "application/dao/channel_dao.hpp"

#include <iostream>

using namespace Pubnub;

Chat::Chat(String publish_key, String subscribe_key, String user_id) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(publish_key, subscribe_key, user_id)
            )
        )
{
    // TODO: This is a hack to get around the fact that we can't call init_services() from the constructor
    const_cast<ChatService*>(chat_service.get())->init_services();

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

Pubnub::Vector<int> Pubnub::Chat::TestVector()
{
    Pubnub::Vector<int> return_vector = {1, 2, 3};

    for(auto elem : return_vector)
    {
        std::cout << "vector: " << elem << std::endl;
    }

    return return_vector;
}

Pubnub::Vector<int> Pubnub::Chat::TestVector2(int param)
{
    Pubnub::Vector<int> return_vector = {1, 2, 3};
    return_vector.push_back(param);

    for(auto elem : return_vector)
    {
        std::cout << "vector: " << elem << std::endl;
    }

    return return_vector;
}

Pubnub::Vector<Pubnub::String> Pubnub::Chat::TestVectorString()
{
    Pubnub::Vector<Pubnub::String> return_vector;
    return_vector.push_back(String("first elem"));
    return_vector.push_back(String("second elem"));
    return return_vector;
}

Channel Chat::create_public_conversation(const String& channel_id, const ChatChannelData& channel_data) const {
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}

CreatedChannelWrapper Chat::create_direct_conversation(const User& user, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_direct_conversation(user, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), std::get<2>(return_tuple));
}

CreatedChannelWrapper Chat::create_group_conversation(const std::vector<User>& users, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_group_conversation(users, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), std::get<2>(return_tuple));
}

Channel Chat::get_channel(const String& channel_id) const
{
    return this->channel_service->get_channel(channel_id);
}

std::vector<Channel> Chat::get_channels(const String& include, int limit, const String& start, const String& end) const {
    return this->channel_service->get_channels(include, limit, start, end);
}

Channel Chat::update_channel(const String& channel_id, const ChatChannelData& channel_data) const {
    return this->channel_service->update_channel(channel_id, channel_data);
}

void Chat::delete_channel(const String& channel_id) const {
    this->channel_service->delete_channel(channel_id);
}

void Chat::pin_message_to_channel(const Message& message, const Channel& channel) const {
    channel.pin_message(message);
}

void Chat::unpin_message_from_channel(const Channel& channel) const {
    channel.unpin_message();
}

User Chat::create_user(const String& user_id, const ChatUserData& user_data) const {
    return this->user_service->create_user(user_id, user_data);
}

User Chat::get_user(const String& user_id) const {
    return this->user_service->get_user(user_id);
}

std::vector<User> Chat::get_users(const String& include, int limit, const String& start, const String& end) const {
    return this->user_service->get_users(include, limit, start, end);
}

User Chat::update_user(const String& user_id, const ChatUserData& user_data) const {
    return this->user_service->update_user(user_id, user_data);
}

void Chat::delete_user(const String& user_id) const {
    return this->user_service->delete_user(user_id);
}

std::vector<String> Chat::where_present(const String& user_id) const {
    return this->presence_service->where_present(user_id);
}

std::vector<String> Chat::who_is_present(const String& channel_id) const {
    return this->presence_service->who_is_present(channel_id);
}

bool Chat::is_present(const String& user_id, const String& channel_id) const {
    return this->presence_service->is_present(user_id, channel_id);
}

void Chat::set_restrictions(const String& user_id, const String& channel_id, const Restriction& restrictions) const {
    this->restrictions_service->set_restrictions(user_id, channel_id, restrictions);
}

void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, const String& channel_id, const String& payload) const {
    this->chat_service->emit_chat_event(chat_event_type, channel_id, payload);
}

void Chat::listen_for_events(const String& channel_id, pubnub_chat_event_type chat_event_type, std::function<void(const String&)> event_callback) const {
    this->chat_service->listen_for_events(channel_id, chat_event_type, event_callback);
}

void Chat::forward_message(const Message& message, const Channel& channel) const {
    this->message_service->forward_message(message, channel.channel_id());
}

std::vector<UnreadMessageWrapper> Chat::get_unread_message_counts(const String& start_timetoken, const String& end_timetoken, const String& filter, int limit)
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

#ifdef PN_CHAT_C_ABI

ChatService* Chat::get_chat_service()
{
    return chat_service.get();
}

#endif
