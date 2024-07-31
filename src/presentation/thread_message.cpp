#include "thread_message.hpp"
#include "channel.hpp"
#include "application/channel_service.hpp"
#include "application/dao/message_dao.hpp"


using namespace Pubnub;


ThreadMessage::ThreadMessage(
        String timetoken, 
        std::shared_ptr<ChatService> chat_service, 
        std::shared_ptr<MessageService> message_service, 
        std::shared_ptr<ChannelService> channel_service, 
        std::shared_ptr<RestrictionsService> restrictions_service,
        std::unique_ptr<MessageDAO> data, 
        String parent_channel_id) :
Message(timetoken, chat_service, message_service, channel_service, restrictions_service, std::move(data)),
parent_channel_id_internal(parent_channel_id)
{}

ThreadMessage::ThreadMessage(
    Pubnub::Message base_message, 
    Pubnub::String parent_channel_id) :
Message(base_message),
parent_channel_id_internal(parent_channel_id)
{}

ThreadMessage& ThreadMessage::operator=(const ThreadMessage& other) {
    this->timetoken_internal = other.timetoken_internal;
    this->chat_service = other.chat_service;
    this->message_service = other.message_service;
    this->channel_service = other.channel_service;
    this->restrictions_service = other.restrictions_service;
    this->data = std::make_unique<MessageDAO>(other.data->to_message_data());
    this->parent_channel_id_internal = other.parent_channel_id_internal;

    return *this;
}

ThreadMessage::~ThreadMessage() = default;

Pubnub::Channel Pubnub::ThreadMessage::pin_to_parent_channel() const
{
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);
    return parent_channel.pin_message(*this);
}

Pubnub::Channel Pubnub::ThreadMessage::unpin_from_parent_channel() const
{
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);
    return parent_channel.unpin_message();
}

