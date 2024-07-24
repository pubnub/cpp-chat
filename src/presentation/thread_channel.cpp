#include "thread_channel.hpp"
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
};