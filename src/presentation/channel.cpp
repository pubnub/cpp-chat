#include "channel.hpp"
#include "application/subscription.hpp"
#include "callback_handle.hpp"
#include "callback_stop.hpp"
#include "chat.hpp"
#include "message.hpp"
#include "application/dao/channel_dao.hpp"
#include "application/channel_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"
#include "application/membership_service.hpp"
#include "message_draft.hpp"
#include "event.hpp"
#include <algorithm>

#ifdef PN_CHAT_C_ABI
#include <pubnub_helper.h>
#endif

using namespace Pubnub;

Channel::Channel() :
    channel_id_internal(""),
    chat_service(nullptr),
    channel_service(nullptr),
    presence_service(nullptr),
    restrictions_service(nullptr),
    message_service(nullptr),
    membership_service(nullptr),
    data(nullptr)
{}

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
}

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

CallbackHandle Channel::connect(std::function<void(Message)> message_callback) const {
    return CallbackHandle(this->channel_service->connect(channel_id_internal, *this->data, message_callback));
}

CallbackHandle Channel::join(std::function<void(Message)> message_callback, const String& additional_params) const {
    return CallbackHandle(this->channel_service->join(*this, *this->data, message_callback, ChatMembershipData{additional_params}));
}

CallbackHandle Channel::join(std::function<void(Message)> message_callback, const ChatMembershipData& membership_data) const {
    return CallbackHandle(this->channel_service->join(*this, *this->data, message_callback, membership_data));
}

void Channel::disconnect() const {
    this->channel_service->disconnect(*this->data);
}

void Channel::leave() const {
    this->channel_service->leave(channel_id_internal, *this->data);
}

#ifdef PN_CHAT_C_ABI
Channel Channel::update_with_base(const Channel& base_channel) const {
    return this->channel_service->update_channel_with_base(*this, base_channel);
}
#endif


void Channel::delete_channel() const {
    this->channel_service->delete_channel(channel_id_internal);
}

void Pubnub::Channel::send_text(const Pubnub::String &message, SendTextParams text_params)
{
    this->channel_service->send_text(channel_id_internal, *this->data, message, SendTextParamsInternal(text_params));
    //Necessary to clean text_params vectors and maps until we find correct way to free Pubnub::Vector data
    text_params.mentioned_users.into_std_map();
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

Restriction Channel::get_user_restrictions(const Pubnub::User& user) const {
    return this->restrictions_service->get_user_restrictions(user.user_id(), channel_id());
}

UsersRestrictionsWrapper Pubnub::Channel::get_users_restrictions(const Pubnub::String &sort, int limit, const Pubnub::Page &page) const
{
    std::tuple<std::vector<Pubnub::UserRestriction>, Pubnub::Page, int, Pubnub::String> return_tuple = this->restrictions_service->get_users_restrictions(channel_id(), sort, limit, page);
    return UsersRestrictionsWrapper({Pubnub::Vector<UserRestriction>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple), std::get<2>(return_tuple), std::get<3>(return_tuple)});
}

Pubnub::Vector<Message> Channel::get_history(const String& start_timetoken, const String& end_timetoken, int count) const {
    return Pubnub::Vector<Message>(std::move(this->channel_service->get_channel_history(channel_id(), start_timetoken, end_timetoken, count)));
}

Message Channel::get_message(const String& timetoken) const {
    Message message = this->message_service->get_message(timetoken, channel_id());
    return message;
}

MembersResponseWrapper Channel::get_members(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    auto return_tuple = this->membership_service->get_channel_members(channel_id(), *this->data, filter, sort, limit, page);
    return MembersResponseWrapper({Pubnub::Vector<Membership>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple), std::get<2>(return_tuple), std::get<3>(return_tuple)});
}

Membership Channel::invite(const User& user) const {
    return this->membership_service->invite_to_channel(channel_id(), *this->data, user);
}

Pubnub::Vector<Membership> Channel::invite_multiple(Pubnub::Vector<User> users) const
{
    auto users_std = users.into_std_vector();
    return Pubnub::Vector<Membership>(std::move(this->membership_service->invite_multiple_to_channel(channel_id(), *this->data, users_std)));
}

void Channel::start_typing() const {
    this->channel_service->start_typing(this->channel_id_internal, *this->data);
}

void Channel::stop_typing() const {
    this->channel_service->stop_typing(this->channel_id_internal, *this->data);
}

CallbackHandle Channel::get_typing(std::function<void(Pubnub::Vector<String>)> typing_callback) const {
    auto new_callback = [=](std::vector<String> vec)
    {
        typing_callback(Pubnub::Vector<String>(std::move(vec)));
    };
    return CallbackHandle(this->channel_service->get_typing(this->channel_id_internal, *this->data, new_callback));
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

CallbackHandle Channel::stream_updates(std::function<void(const Channel&)> channel_callback) const {
    return CallbackHandle(this->channel_service->stream_updates(*this, channel_callback));
}

CallbackHandle Channel::stream_updates_on(Pubnub::Vector<Channel> channels, std::function<void(Pubnub::Vector<Pubnub::Channel>)> channel_callback) {
    auto channels_std = channels.into_std_vector();

    auto new_callback = [channel_callback] (std::vector<Pubnub::Channel> vec)
    {
        channel_callback(std::move(vec));
    };

    return CallbackHandle(this->channel_service->stream_updates_on(*this, channels_std, new_callback));
}

CallbackHandle Channel::stream_presence(std::function<void(Pubnub::Vector<String>)> presence_callback) const {
    auto new_callback = [=](std::vector<String> vec)
    {
        presence_callback(Pubnub::Vector<String>(std::move(vec)));
    };

    return CallbackHandle(this->presence_service->stream_presence(channel_id(), new_callback));
}

CallbackHandle Pubnub::Channel::stream_read_receipts(std::function<void(Pubnub::Map<Pubnub::String, Pubnub::Vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const
{
    auto new_callback = [=](std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer> std_map)
    {
        std::map<Pubnub::String, Pubnub::Vector<Pubnub::String>, Pubnub::StringComparer> tmp_map;
        for(auto it = std_map.begin(); it != std_map.end(); it++)
        {
            Pubnub::Vector<Pubnub::String> vec(std::move(it->second));
            tmp_map[it->first] = vec;
        }

        Pubnub::Map<Pubnub::String, Pubnub::Vector<Pubnub::String>, Pubnub::StringComparer> final_map = tmp_map;
        read_receipts_callback(final_map);
    };

    return CallbackHandle(this->channel_service->stream_read_receipts(channel_id(), *this->data, new_callback));

}

void Channel::forward_message(const Message& message) const {
    this->message_service->forward_message(message, channel_id_internal);
}

void Pubnub::Channel::emit_user_mention(const Pubnub::String &user_id, const Pubnub::String &timetoken, const Pubnub::String &text) const
{
    this->channel_service->emit_user_mention(channel_id(), user_id, timetoken, text);
}

Pubnub::Vector<Pubnub::Membership> Pubnub::Channel::get_user_suggestions(Pubnub::String text, int limit) const
{
    return Pubnub::Vector<Membership>(std::move(this->channel_service->get_user_suggestions_for_channel(channel_id(), *this->data, text, limit)));
}

Pubnub::EventsHistoryWrapper Pubnub::Channel::get_messsage_reports_history(const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count) const
{
    auto return_tuple = this->channel_service->get_message_reports_history(channel_id(), start_timetoken, end_timetoken, count);
    return EventsHistoryWrapper({Pubnub::Vector<Event>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple)});
}

Pubnub::MessageDraft Pubnub::Channel::create_message_draft(Pubnub::MessageDraftConfig message_draft_config) const
{
    return this->message_service->create_message_draft(*this, message_draft_config);
}

Pubnub::CallbackHandle Pubnub::Channel::stream_message_reports(std::function<void(const Pubnub::Event&)> event_callback) const
{
    // TODO: it seems to be bug
    auto new_callback = [=](const Event& event)
    {
        event_callback(event);
    };
    return CallbackHandle(this->channel_service->stream_message_reports(channel_id(), new_callback));
}

#ifdef PN_CHAT_C_ABI
std::shared_ptr<const ChatService> Channel::shared_chat_service() const
{
    return this->chat_service;
}
#endif
