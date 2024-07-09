#include "presentation/message.hpp"
#include "application/message_service.hpp"
#include "application/channel_service.hpp"
#include "application/restrictions_service.hpp"

using namespace Pubnub;

Message::Message(String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<ChannelService> channel_service,
                    std::shared_ptr<RestrictionsService> restrictions_service) :
timetoken_internal(timetoken),
chat_service(chat_service),
message_service(message_service),
channel_service(channel_service),
restrictions_service(restrictions_service)
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

pubnub_chat_message_type Message::type()
{
    return this->message_data().type;
}

void Message::pin()
{
    Channel channel = this->channel_service->get_channel(message_data().channel_id);
    this->channel_service->pin_message_to_channel(*this, channel);
}

Message Message::toggle_reaction(String reaction)
{
    this->message_service->toggle_reaction(*this, reaction);
    return *this;
}

std::vector<MessageAction> Message::reactions()
{
    return this->message_service->get_message_reactions(*this);
}

bool Message::has_user_reaction(String reaction)
{
    auto message_reactions = this->message_service->get_message_reactions(*this);
    for (auto message_reaction : message_reactions)
    {
        if(message_reaction.value == reaction)
        {
            return true;
        }
    }

    return false;
}

void Message::forward(String channel_id)
{
    this->message_service->forward_message(*this, channel_id);
}

void Message::report(Pubnub::String reason)
{
    this->restrictions_service->report_message(*this, reason);
}

void Message::stream_updates(std::function<void(Message)> message_callback)
{
    this->message_service->stream_updates_on({*this}, message_callback);
}

void Message::stream_updates_on(std::vector<Message> messages, std::function<void(Message)> message_callback)
{
    this->message_service->stream_updates_on(messages, message_callback);
}