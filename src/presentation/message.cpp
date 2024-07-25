#include "message.hpp"
#include "application/message_service.hpp"
#include "application/channel_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/dao/message_dao.hpp"

using namespace Pubnub;

Message::Message(String timetoken, std::shared_ptr<const ChatService> chat_service, std::shared_ptr<const MessageService> message_service, std::shared_ptr<const ChannelService> channel_service, std::shared_ptr<const RestrictionsService> restrictions_service, std::unique_ptr<MessageDAO> data) :
timetoken_internal(timetoken),
chat_service(chat_service),
message_service(message_service),
channel_service(channel_service),
restrictions_service(restrictions_service),
data(std::move(data))
{}

Message::Message(const Message& other) :
timetoken_internal(other.timetoken_internal),
chat_service(other.chat_service),
message_service(other.message_service),
channel_service(other.channel_service),
restrictions_service(other.restrictions_service),
data(std::make_unique<MessageDAO>(other.data->to_message_data()))
{}

Message& Message::operator =(const Message& other) {
    if(this == &other)
    {
        return *this;
    }

    this->timetoken_internal = other.timetoken_internal;
    this->chat_service = other.chat_service;
    this->message_service = other.message_service;
    this->channel_service = other.channel_service;
    this->restrictions_service = other.restrictions_service;
    this->data = std::make_unique<MessageDAO>(other.data->to_message_data());

    return *this;
}

Message::~Message() = default;

String Message::timetoken() const {
    return this->timetoken_internal;
}

ChatMessageData Message::message_data() const {
    return this->data->to_message_data();
}

Message Message::edit_text(const String& new_text) const {
    return this->message_service->edit_text(this->timetoken(), *this->data, new_text);
}

String Message::text() const {
    return this->message_service->text(*this->data);
}

Message Message::delete_message() const {
    return this->message_service->delete_message(*this->data, this->timetoken());
}

bool Message::deleted() const {
    return this->message_service->deleted(*this->data);
}

pubnub_chat_message_type Message::type() const {
    return this->message_data().type;
}

void Message::pin() const {
    Channel channel = this->channel_service->get_channel(message_data().channel_id);
    channel.pin_message(*this);
}

Message Message::toggle_reaction(const String& reaction) const {
    return this->message_service->toggle_reaction(this->timetoken(), *this->data, reaction);
}

std::vector<MessageAction> Message::reactions() const {
    return this->message_service->get_message_reactions(*this->data);
}

bool Message::has_user_reaction(const String& reaction) const {
    auto message_reactions = this->message_service->get_message_reactions(*this->data);
    for (auto message_reaction : message_reactions)
    {
        if(message_reaction.value == reaction)
        {
            return true;
        }
    }

    return false;
}

void Message::forward(const String& channel_id) const {
    this->message_service->forward_message(*this, channel_id);
}

void Message::report(const Pubnub::String& reason) const {
    this->restrictions_service->report_message(*this, reason);
}

void Message::stream_updates(std::function<void(const Message&)> message_callback) const {
    this->message_service->stream_updates_on({*this}, message_callback);
}

void Message::stream_updates_on(const std::vector<Message>& messages, std::function<void(const Message&)> message_callback) const {
    this->message_service->stream_updates_on(messages, message_callback);
}
