#include "thread_message.hpp"
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
internal_parent_channel_id(parent_channel_id)
{}

ThreadMessage::ThreadMessage(
    Pubnub::Message base_message, 
    Pubnub::String parent_channel_id) :
Message(base_message),
internal_parent_channel_id(parent_channel_id)
{}

ThreadMessage& ThreadMessage::operator=(const ThreadMessage& other) {
    this->timetoken_internal = other.timetoken_internal;
    this->chat_service = other.chat_service;
    this->message_service = other.message_service;
    this->channel_service = other.channel_service;
    this->restrictions_service = other.restrictions_service;
    this->data = std::make_unique<MessageDAO>(other.data->to_message_data());
    this->internal_parent_channel_id = other.internal_parent_channel_id;

    return *this;
}

ThreadMessage::~ThreadMessage() = default;