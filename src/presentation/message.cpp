#include "message.hpp"

#include "application/channel_service.hpp"
#include "application/dao/message_dao.hpp"
#include "application/message_service.hpp"
#include "application/restrictions_service.hpp"
#include "base_message.hpp"
#include "thread_channel.hpp"

using namespace Pubnub;

Message::Message() : BaseMessage<Message>() {}

Message::Message(
    String timetoken,
    std::shared_ptr<const ChatService> chat_service,
    std::shared_ptr<const MessageService> message_service,
    std::shared_ptr<const ChannelService> channel_service,
    std::shared_ptr<const RestrictionsService> restrictions_service,
    std::unique_ptr<MessageDAO> data
) :
    BaseMessage<Message>(
        timetoken,
        chat_service,
        message_service,
        channel_service,
        restrictions_service,
        std::move(data)
    ) {}

Message::Message(const Message& other) : BaseMessage<Message>(other) {}

Message Message::edit_text(const String& new_text) const {
    return this->message_service->edit_text(this->timetoken(), *this->data, new_text);
}

Message Message::delete_message() const {
    return this->message_service->delete_message(*this->data, this->timetoken());
}

Message Message::restore() const {
    return this->message_service->restore(*this->data, this->timetoken());
}

Message Message::toggle_reaction(const String& reaction) const {
    return this->message_service->toggle_reaction(this->timetoken(), *this->data, reaction);
}

CallbackStop Message::stream_updates(std::function<void(const Message&)> message_callback) const {
    return CallbackStop(
        this->message_service->stream_updates(*this, message_callback)
    );
}

CallbackStop Message::stream_updates_on(
    Pubnub::Vector<Pubnub::Message> messages,
    std::function<void(Pubnub::Vector<Pubnub::Message>)> message_callback
) const {
    auto messages_std = messages.into_std_vector();

    auto new_callback = [=](std::vector<Pubnub::Message> vec) { message_callback(std::move(vec)); };
    return CallbackStop(this->message_service->stream_updates_on(*this, messages_std, new_callback)
    );
}

Pubnub::Option<Pubnub::Message> Pubnub::Message::quoted_message() const {
    QuotedMessage quoted_message = this->data->to_entity().get_quoted_message();

    if (quoted_message.timetoken.empty()) {
        return Pubnub::Option<Pubnub::Message>();
    }

    try {
        auto channel = this->channel_service->get_channel(message_data().channel_id);
        Pubnub::Message final_quoted_message = channel.get_message(quoted_message.timetoken);
        return final_quoted_message;

    } catch (...) {
        return Pubnub::Option<Pubnub::Message>();
    }

    return Pubnub::Option<Pubnub::Message>();
}

    ThreadChannel Message::create_thread() const {
        return this->channel_service->create_thread_channel(*this);
    }

    ThreadChannel Message::get_thread() const {
        return this->channel_service->get_thread_channel(*this);
    }



#ifdef PN_CHAT_C_ABI
Pubnub::Message Pubnub::Message::update_with_base(const Pubnub::Message& base_message) const {
    return this->message_service->update_message_with_base(*this, base_message);
}
#endif
