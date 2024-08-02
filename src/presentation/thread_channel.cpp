#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "application/channel_service.hpp"
#include "application/dao/channel_dao.hpp"


using namespace Pubnub;

ThreadChannel::ThreadChannel(
                    String channel_id, 
                    std::shared_ptr<const ChatService> chat_service,
                    std::shared_ptr<const ChannelService> channel_service,
                    std::shared_ptr<const PresenceService> presence_service,
                    std::shared_ptr<const RestrictionsService> restrictions_service,
                    std::shared_ptr<const MessageService> message_service,
                    std::shared_ptr<const MembershipService> membership_service,
                    std::unique_ptr<ChannelDAO> data,
                    String parent_channel_id, 
                    Message parent_message) :
Channel(channel_id, chat_service, channel_service, presence_service, restrictions_service, message_service, membership_service, std::move(data)),
parent_channel_id_internal(parent_channel_id),
parent_message_internal(parent_message)
{}


ThreadChannel& ThreadChannel::operator =(const ThreadChannel& other)
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
    this->parent_channel_id_internal = other.parent_channel_id_internal;
    this->parent_message_internal = other.parent_message_internal;

    return *this;
}

ThreadChannel::~ThreadChannel() = default;

void ThreadChannel::send_text(const String &message, pubnub_chat_message_type message_type, const String &meta_data)
{
    //If this is new thread, set all server data before sending the first message (this is actually creating the thread, even if the object was created earlier)
    if(!is_thread_created)
    {
        this->channel_service->confirm_creating_thread(*this);
        is_thread_created = true;
    }

    Channel::send_text(message, message_type, meta_data);
}

Pubnub::Vector<Pubnub::ThreadMessage> Pubnub::ThreadChannel::get_thread_history(const Pubnub::String &start_timetoken, const Pubnub::String &end_timetoken, int count) const
{
    return Pubnub::Vector<ThreadMessage>(std::move(this->channel_service->get_thread_channel_history(channel_id(), start_timetoken, end_timetoken, count, parent_channel_id())));
}

Pubnub::ThreadChannel Pubnub::ThreadChannel::pin_message_to_thread(const Pubnub::ThreadMessage &message) const
{
    return this->channel_service->pin_message_to_thread_channel(message, *this);
}

Pubnub::ThreadChannel Pubnub::ThreadChannel::unpin_message_from_thread() const
{
    return this->channel_service->unpin_message_from_thread_channel(*this);
}

Pubnub::Channel Pubnub::ThreadChannel::pin_message_to_parent_channel(const Pubnub::ThreadMessage &message) const
{
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);
    return parent_channel.pin_message(message);
}

Pubnub::Channel Pubnub::ThreadChannel::unpin_message_from_parent_channel() const
{
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);
    return parent_channel.unpin_message();
}

void Pubnub::ThreadChannel::emit_user_mention(const Pubnub::String &user_id, const Pubnub::String &timetoken, const Pubnub::String &text) const
{
    this->channel_service->emit_user_mention(channel_id(), user_id, timetoken, text, parent_channel_id_internal);
}

void ThreadChannel::set_is_thread_created(bool is_created) 
{
    this->is_thread_created = is_created;
};