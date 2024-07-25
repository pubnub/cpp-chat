#include "channel.hpp"
#include "message.hpp"
#include "application/dao/channel_dao.hpp"
#include "application/channel_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"
#include "application/membership_service.hpp"
#include <algorithm>

using namespace Pubnub;

Channel::Channel(
        String channel_id,
        std::shared_ptr<const ChatService> chat_service,
        std::shared_ptr<const ChannelService> channel_service,
        std::shared_ptr<const PresenceService> presence_service,
        std::shared_ptr<const RestrictionsService> restrictions_service,
        std::shared_ptr<const MessageService> message_service,
        std::shared_ptr<const MembershipService> membership_service,
        std::unique_ptr<ChannelDAO> data) :
    channel_id_internal(channel_id),
    chat_service(chat_service),
    channel_service(channel_service),
    presence_service(presence_service),
    restrictions_service(restrictions_service),
    message_service(message_service),
    membership_service(membership_service),
    data(std::move(data))
{}

Channel::Channel(const Channel& other) :
    channel_id_internal(other.channel_id_internal),
    chat_service(other.chat_service),
    channel_service(other.channel_service),
    presence_service(other.presence_service),
    restrictions_service(other.restrictions_service),
    message_service(other.message_service),
    membership_service(other.membership_service),
    data(std::make_unique<ChannelDAO>(other.data->to_channel_data()))
{}

Channel& Channel::operator =(const Channel& other)
{
    if(this == &other)
    {
        return *this;
    }
    this->channel_id_internal = other.channel_id_internal;
    this->data = std::make_unique<::ChannelDAO>(other.data->to_channel_data());
    this->channel_service = other.channel_service;
    this->chat_service = other.chat_service;
    this->presence_service = other.presence_service;
    this->restrictions_service = other.restrictions_service;
    this->message_service = other.message_service;
    this->membership_service = other.membership_service;

    return *this;
};

Channel::~Channel() = default;

String Channel::channel_id() const {
    return this->channel_id_internal;
}

ChatChannelData Channel::channel_data() const {
    return this->data->to_channel_data();
}

Channel Channel::update(const ChatChannelData& in_additional_channel_data) const {
    return this->channel_service->update_channel(channel_id_internal, ChannelDAO(in_additional_channel_data));
}

void Channel::connect(std::function<void(Message)> message_callback) const {
    this->channel_service->connect(channel_id_internal, message_callback);
}

void Channel::disconnect() const {
    this->channel_service->disconnect(channel_id_internal);
}

void Channel::join(std::function<void(Message)> message_callback, const String& additional_params) const {
    this->channel_service->join(channel_id_internal, message_callback, additional_params);
}

void Channel::leave() const {
    this->channel_service->leave(channel_id_internal);
}

void Channel::delete_channel() const {
    this->channel_service->delete_channel(channel_id_internal);
}

void Channel::send_text(const String& message, pubnub_chat_message_type message_type, const String& meta_data) const {
    this->channel_service->send_text(channel_id_internal, message, message_type, meta_data);
}

Pubnub::Vector<String> Channel::who_is_present() const {
    return Pubnub::Vector<String>(std::move(this->presence_service->who_is_present(channel_id_internal)));
}

bool Channel::is_present(const String& user_id) const {
    return this->presence_service->is_present(user_id, channel_id_internal);
}

void Channel::set_restrictions(const String& user_id, Restriction restrictions) const {
    this->restrictions_service->set_restrictions(user_id, channel_id_internal, restrictions);
}

Restriction Channel::get_user_restrictions(const String& user_id, const String& channel_id, int limit, const String& start, const String& end) const {
    return this->restrictions_service->get_user_restrictions(user_id, channel_id, limit, start, end);
}

Pubnub::Vector<Message> Channel::get_history(const String& start_timetoken, const String& end_timetoken, int count) const {
    return Pubnub::Vector<Message>(std::move(this->message_service->get_channel_history(channel_id(), start_timetoken, end_timetoken, count)));
}

Message Channel::get_message(const String& timetoken) const {
    Message message = this->message_service->get_message(timetoken, channel_id());
    return message;
}

Pubnub::Vector<Membership> Channel::get_members(int limit, const String& start_timetoken, const String& end_timetoken) const {
    return Pubnub::Vector<Membership>(std::move(this->membership_service->get_channel_members(channel_id(), *this->data, limit, start_timetoken, end_timetoken)));
}

Membership Channel::invite(const User& user) const {
    return this->membership_service->invite_to_channel(channel_id(), *this->data, user);
}

Pubnub::Vector<Membership> Channel::invite_multiple(Pubnub::Vector<User> users) const
{
    return Pubnub::Vector<Membership>(std::move(this->membership_service->invite_multiple_to_channel(channel_id(), *this->data, users.into_std_vector())));
}

void Channel::start_typing() const {
    this->channel_service->start_typing(this->channel_id_internal, *this->data);
}

void Channel::stop_typing() const {
    this->channel_service->stop_typing(this->channel_id_internal, *this->data);
}

void Channel::get_typing(std::function<void(Pubnub::Vector<String>)> typing_callback) const {
    auto new_callback = [=](std::vector<String> vec)
    {
        typing_callback(Pubnub::Vector<String>(std::move(vec)));
    };
    this->channel_service->get_typing(this->channel_id_internal, *this->data, new_callback);
}

Channel Channel::pin_message(const Message& message) const {
    return this->channel_service->pin_message_to_channel(message, this->channel_id_internal, *this->data);
}

Channel Channel::unpin_message() const {
    return this->channel_service->unpin_message_from_channel(this->channel_id_internal, *this->data);
}

Message Channel::get_pinned_message() const {
    return this->channel_service->get_pinned_message(this->channel_id_internal, *this->data);
}

void Channel::stream_updates(std::function<void(const Channel&)> channel_callback) const {
    this->channel_service->stream_updates_on({this->channel_id()}, channel_callback);
}

void Channel::stream_updates_on(Pubnub::Vector<Channel> channels, std::function<void(const Channel&)> channel_callback) const {
    
    auto channels_std = channels.into_std_vector();
    std::vector<String> channel_ids(channels_std.size());
    std::transform(
            channels_std.begin(),
            channels_std.end(),
            channel_ids.begin(),
            [](Channel channel) { return channel.channel_id(); }
        );
    this->channel_service->stream_updates_on(channel_ids, channel_callback);
}

void Channel::stream_presence(std::function<void(Pubnub::Vector<String>)> presence_callback) const {
    auto new_callback = [=](std::vector<String> vec)
    {
        presence_callback(Pubnub::Vector<String>(std::move(vec)));
    };
    this->presence_service->stream_presence(channel_id(), new_callback);
}
void Channel::forward_message(const Message& message) const {
    this->message_service->forward_message(message, channel_id_internal);
}
