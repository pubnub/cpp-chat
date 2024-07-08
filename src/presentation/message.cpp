#include "presentation/message.hpp"
#include "application/message_service.hpp"

using namespace Pubnub;

Message::Message(String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service) :
timetoken_internal(timetoken),
chat_service(chat_service),
message_service(message_service)
{}

ChatMessageData Message::message_data()
{
    return this->message_service->get_message_data(timetoken());
}

Message Message::edit_text(String new_text)
{
    this->message_service->edit_text(*this, new_text);
    return *this;
}

String Message::text()
{
    return this->message_service->text(*this);
}

Message Message::delete_message()
{
    this->message_service->delete_message(*this);
    return *this;
}

bool Message::deleted()
{
    return this->message_service->deleted(*this);
}

void Message::pin()
{

}

void Message::stream_updates(std::function<void(Message)> message_callback)
{
    this->message_service->stream_updates_on({*this}, message_callback);
}

void Message::stream_updates_on(std::vector<Message> messages, std::function<void(Message)> message_callback)
{
    this->message_service->stream_updates_on(messages, message_callback);
}