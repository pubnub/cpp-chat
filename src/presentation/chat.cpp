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
#include "callback_handle.hpp"
#include "enums.hpp"
#include "mentions.hpp"
#include <algorithm>
#include <vector>

extern "C" {
    #include <pubnub_subscribe_v2.h>
#ifdef PN_CHAT_C_ABI
    #include <pubnub_helper.h>
#endif
}

#include <iostream>

using namespace Pubnub;

Chat::Chat(const Pubnub::String& publish_key, const Pubnub::String& subscribe_key, const Pubnub::String& user_id, const ChatConfig& config) :
    chat_service(
            std::make_shared<ChatService>(
                ChatService::create_pubnub(publish_key, subscribe_key, user_id, config.auth_key)
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
    callback_service = chat_service->callback_service;
}

Chat Chat::init(const Pubnub::String& publish_key, const Pubnub::String& subscribe_key, const Pubnub::String& user_id, const ChatConfig &config)
{
    if (config.store_user_activity_interval < 60000) {
        throw std::invalid_argument("store_user_activity_interval must be at least 60000ms");
    }

    Chat chat(publish_key, subscribe_key, user_id, config);
    try
    {
        chat.get_user(user_id);
    }
    catch (...)
    {
        chat.create_user_for_init_chat(user_id, ChatUserData());
    }

    if (config.store_user_activity_timestamps) {
        chat.store_user_activity_timestamp();
    }
    
    return chat;
}

Channel Chat::create_public_conversation(const String& channel_id, const ChatChannelData& channel_data) const {
    return this->channel_service->create_public_conversation(channel_id, channel_data);
}

CreatedChannelWrapper Chat::create_direct_conversation(const User& user, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_direct_conversation(user, channel_id, channel_data, ChatMembershipData{membership_data});
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

CreatedChannelWrapper Chat::create_direct_conversation(const User& user, const String& channel_id, const ChatChannelData& channel_data, const ChatMembershipData& membership_data) const {
    auto return_tuple = this->channel_service->create_direct_conversation(user, channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

CreatedChannelWrapper Chat::create_group_conversation(Pubnub::Vector<User> users, const String& channel_id, const ChatChannelData& channel_data, const String& membership_data) const {
    auto return_tuple = this->channel_service->create_group_conversation(users.into_std_vector(), channel_id, channel_data, Pubnub::ChatMembershipData{membership_data});
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

CreatedChannelWrapper Chat::create_group_conversation(Pubnub::Vector<User> users, const String& channel_id, const ChatChannelData& channel_data, const ChatMembershipData& membership_data) const {
    auto return_tuple = this->channel_service->create_group_conversation(users.into_std_vector(), channel_id, channel_data, membership_data);
    return CreatedChannelWrapper(std::get<0>(return_tuple), std::get<1>(return_tuple), Pubnub::Vector<Membership>(std::move(std::get<2>(return_tuple))));
}

Channel Chat::get_channel(const String& channel_id) const
{
    return this->channel_service->get_channel(channel_id);
}

ChannelsResponseWrapper Chat::get_channels(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    auto return_tuple = this->channel_service->get_channels(filter, sort, limit, page);
    return ChannelsResponseWrapper({Pubnub::Vector<Channel>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple), std::get<2>(return_tuple)});
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

Pubnub::User Pubnub::Chat::current_user()
{
    return this->user_service->get_current_user();
}

User Chat::create_user(const String &user_id, const ChatUserData &user_data) const
{
    return this->user_service->create_user(user_id, user_data);
}

User Chat::get_user(const String& user_id) const {
    return this->user_service->get_user(user_id);
}

UsersResponseWrapper Chat::get_users(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    auto return_tuple = this->user_service->get_users(filter, sort, limit, page);
    return UsersResponseWrapper({Pubnub::Vector<User>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple), std::get<2>(return_tuple)});
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

void Chat::emit_chat_event(pubnub_chat_event_type chat_event_type, const String& channel_id, const String& payload, EventMethod event_method) const {
    this->chat_service->emit_chat_event(chat_event_type, channel_id, payload, event_method);
}

EventsHistoryWrapper Pubnub::Chat::get_events_history(const Pubnub::String &channel_id, const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count) const
{
    auto return_tuple = this->chat_service->get_events_history(channel_id, start_timetoken, end_timetoken, count);
    return EventsHistoryWrapper({Pubnub::Vector<Event>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple)});
}

CallbackHandle Chat::listen_for_events(const String& channel_id, pubnub_chat_event_type chat_event_type, std::function<void(const Event&)> event_callback) const {
    return CallbackHandle(this->chat_service->listen_for_events(channel_id, chat_event_type, event_callback));
}

void Chat::forward_message(const Message& message, const Channel& channel) const {
    this->message_service->forward_message(message, channel.channel_id());
}

Pubnub::Vector<UnreadMessageWrapper> Chat::get_unread_messages_counts(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const
{
    auto tuples = this->membership_service->get_all_unread_messages_counts(filter, sort, limit, page);

    std::vector<UnreadMessageWrapper> return_wrappers;
    for(auto &tuple : tuples)
    {
        UnreadMessageWrapper wrapper = {std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)};
        return_wrappers.push_back(wrapper);
    }

    return_wrappers.erase(
            std::remove_if(
                return_wrappers.begin(),
                return_wrappers.end(),
                [](const UnreadMessageWrapper& wrapper) { return wrapper.count <= 0; }),
            return_wrappers.end());

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

UserMentionDataList Pubnub::Chat::get_current_user_mentions(const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count) const
{
    auto return_tuple = this->chat_service->get_current_user_mentions(start_timetoken, end_timetoken, count);

    UserMentionDataList return_list;
    return_list.user_mention_data = Pubnub::Vector<Pubnub::UserMentionData>(std::move(std::get<0>(return_tuple)));
    return_list.is_more = std::get<1>(return_tuple);

    return return_list;
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

AccessManager Chat::access_manager() const
{
    return AccessManager(this->chat_service->access_manager_service);
}

void Pubnub::Chat::register_logger_callback(std::function<void(Pubnub::pn_log_level, const char*)> callback) 
{
    this->chat_service->register_logger_callback(callback);
}

void Chat::store_user_activity_timestamp() const
{
    this->user_service->store_user_activity_timestamp();
}

Pubnub::User Chat::create_user_for_init_chat(const Pubnub::String& user_id, const Pubnub::ChatUserData& user_data) const
{
    //During init we skip get_user during create_user, as we already checked that the user doesn't exist
    return this->user_service->create_user(user_id, user_data, true);
}

void Chat::add_connection_status_listener(std::function<void(Pubnub::pn_connection_status status, Pubnub::ConnectionStatusData status_data)> connection_status_callback) const
{
    this->chat_service->add_connection_status_listener(connection_status_callback);
}

bool Chat::reconnect_subscriptions() const
{
    return this->chat_service->reconnect_subscriptions();
}

void Chat::remove_connection_status_listener() const
{
    this->chat_service->remove_connection_status_listener();
}

bool Chat::disconnect_subscriptions() const
{
    return this->chat_service->disconnect_subscriptions();
}

#ifdef PN_CHAT_C_ABI

const ChatService* Chat::get_chat_service() const {
    return chat_service.get();
}

std::shared_ptr<const ChatService> Chat::shared_chat_service() const {
    return chat_service;
}

#endif
