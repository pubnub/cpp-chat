#include "thread_channel.hpp"
#include "application/channel_service.hpp"
#include "application/dao/channel_dao.hpp"


using namespace Pubnub;

ThreadChannel::ThreadChannel(
                    Pubnub::String channel_id, 
                    std::shared_ptr<const ChatService> chat_service,
                    std::shared_ptr<const ChannelService> channel_service,
                    std::shared_ptr<const PresenceService> presence_service,
                    std::shared_ptr<const RestrictionsService> restrictions_service,
                    std::shared_ptr<const MessageService> message_service,
                    std::shared_ptr<const MembershipService> membership_service,
                    std::unique_ptr<ChannelDAO> data,
                    Pubnub::String parent_channel_id, 
                    Pubnub::Message parent_message) :
Channel(channel_id, chat_service, channel_service, presence_service, restrictions_service, message_service, membership_service, std::move(data)),
parent_channel_id(parent_channel_id),
parent_message(parent_message)
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
    this->parent_channel_id = other.parent_channel_id;
    this->parent_message = other.parent_message;

    return *this;
}

void Pubnub::ThreadChannel::send_text(const Pubnub::String &message, Pubnub::pubnub_chat_message_type message_type, const Pubnub::String &meta_data) const
{
    //If this is new thread, set all server data before sending the first message (this is actually creating the thread, even if the object was created earlier)
    if(!is_thread_created)
    {
        this->channel_service->confirm_creating_thread(*this);
        is_thread_created = true;
    }

    Channel::send_text(message, message_type, meta_data);
}

void Pubnub::ThreadChannel::set_is_thread_created(bool is_created) 
{
    this->is_thread_created = is_created;
};