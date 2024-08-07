#include "chat.hpp"
#include "application/chat_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"
#include "application/membership_service.hpp"
#include "application/dao/channel_dao.hpp"
#include "application/access_manager_service.hpp"
#include <vector>

extern "C" {
    #include <pubnub_subscribe_v2.h>
#ifdef PN_CHAT_C_ABI
    #include <pubnub_helper.h>
#endif
}


#include <iostream>

using namespace Pubnub;

Chat::Chat(const ChatConfig& config) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(config.publish_key, config.subscribe_key, config.user_id)
            )
        )
{
    // TODO: This is a hack to get around the fact that we can't call init_services() from the constructor
    const_cast<ChatService*>(chat_service.get())->init_services(config);

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

Channel Chat::create_public_conversation(const String& channel_id, const ChatChannelData& channel_data) const {
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}

CreatedChannelWrapper Chat::create_direct_conversation(const User& user, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_direct_conversation(user, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

CreatedChannelWrapper Chat::create_group_conversation(Pubnub::Vector<User> users, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_group_conversation(users.into_std_vector(), channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

Channel Chat::get_channel(const String& channel_id) const
{
    return this->channel_service->get_channel(channel_id);
}

Pubnub::Vector<Channel> Chat::get_channels(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    return Pubnub::Vector<Channel>(std::move(this->channel_service->get_channels(filter, sort, limit, page)));
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

Pubnub::Vector<Channel> Pubnub::Chat::get_channel_suggestions(String text, int limit) const
{
    return Pubnub::Vector<Channel>(std::move(this->channel_service->get_channel_suggestions(text, limit)));
}

User Chat::create_user(const String& user_id, const ChatUserData& user_data) const {
    return this->user_service->create_user(user_id, user_data);
}

User Chat::get_user(const String& user_id) const {
    return this->user_service->get_user(user_id);
}

Pubnub::Vector<User> Chat::get_users(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    return Pubnub::Vector<User>(std::move(this->user_service->get_users(filter, sort, limit, page)));
}

User Chat::update_user(const String& user_id, const ChatUserData& user_data) const {
    return this->user_service->update_user(user_id, user_data);
}

void Chat::delete_user(const String& user_id) const {
    return this->user_service->delete_user(user_id);
}

Pubnub::Vector<User> Chat::get_user_suggestions(String text, int limit) const
{
    return Pubnub::Vector<User>(std::move(this->user_service->get_users_suggestions(text, limit)));
}

Pubnub::Vector<String> Chat::where_present(const String& user_id) const {
    return Pubnub::Vector<String>(std::move(this->presence_service->where_present(user_id)));
}

Pubnub::Vector<String> Chat::who_is_present(const String& channel_id) const {
    return Pubnub::Vector<String>(std::move(this->presence_service->who_is_present(channel_id)));
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

#ifndef PN_CHAT_C_ABI
CallbackStop Chat::listen_for_events(const String& channel_id, pubnub_chat_event_type chat_event_type, std::function<void(const Event&)> event_callback) const {
    return CallbackStop(this->chat_service->listen_for_events(channel_id, chat_event_type, event_callback));
}
#else
std::vector<pubnub_v2_message> Chat::listen_for_events(const String& channel_id, pubnub_chat_event_type chat_event_type) const {
    return this->chat_service->listen_for_events(channel_id, chat_event_type);
}
#endif

void Chat::forward_message(const Message& message, const Channel& channel) const {
    this->message_service->forward_message(message, channel.channel_id());
}

Pubnub::Vector<UnreadMessageWrapper> Chat::get_unread_messages_counts(const String& start_timetoken, const String& end_timetoken, const String& filter, int limit) const
{
    auto tuples = this->membership_service->get_all_unread_messages_counts(start_timetoken, end_timetoken, filter, limit);

    std::vector<UnreadMessageWrapper> return_wrappers;
    for(auto &tuple : tuples)
    {
        UnreadMessageWrapper wrapper = {std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)};
        return_wrappers.push_back(wrapper);
    }

    return Pubnub::Vector<UnreadMessageWrapper>(std::move(return_wrappers));
}

MarkMessagesAsReadWrapper Pubnub::Chat::mark_all_messages_as_read(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const
{
    auto return_tuple = this->membership_service->mark_all_messages_as_read(filter, sort, limit, page);
    MarkMessagesAsReadWrapper Wrapper;
    Wrapper.page = std::get<0>(return_tuple);
    Wrapper.total = std::get<1>(return_tuple);
    Wrapper.status = std::get<2>(return_tuple);
    Wrapper.memberships = std::move(std::get<3>(return_tuple));

    return Wrapper;
}

ThreadChannel Chat::create_thread_channel(const Pubnub::Message& message) const
{
    return this->channel_service->create_thread_channel(message);
}

ThreadChannel Chat::get_thread_channel(const Pubnub::Message& message) const
{
    return this->channel_service->get_thread_channel(message);
}

void Chat::remove_thread_channel(const Pubnub::Message& message) const
{
    this->channel_service->remove_thread_channel(message);
}

#ifdef PN_CHAT_C_ABI

const ChatService* Chat::get_chat_service() const {
    return chat_service.get();
}

std::vector<pubnub_v2_message> Chat::get_chat_updates() const
{
    return this->chat_service->get_chat_updates();
}

#endif
