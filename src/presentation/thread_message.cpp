#include "thread_message.hpp"

#include "application/channel_service.hpp"
#include "application/dao/message_dao.hpp"
#include "application/message_service.hpp"
#include "channel.hpp"
#include "thread_channel.hpp"

using namespace Pubnub;

ThreadMessage::ThreadMessage(
    String timetoken,
    std::shared_ptr<ChatService> chat_service,
    std::shared_ptr<MessageService> message_service,
    std::shared_ptr<ChannelService> channel_service,
    std::shared_ptr<RestrictionsService> restrictions_service,
    std::unique_ptr<MessageDAO> data,
    String parent_channel_id
) :
    BaseMessage(
        timetoken,
        chat_service,
        message_service,
        channel_service,
        restrictions_service,
        std::move(data)
    ),
    parent_channel_id_internal(parent_channel_id) {}

// TODO:???
//ThreadMessage::ThreadMessage(
//    Pubnub::Message base_message,
//    Pubnub::String parent_channel_id) :
//BaseMessage(base_message),
//parent_channel_id_internal(parent_channel_id)
//{}

ThreadMessage& ThreadMessage::operator=(const ThreadMessage& other) {
    BaseMessage::operator=(other);
    this->parent_channel_id_internal = other.parent_channel_id_internal;

    return *this;
}

ThreadMessage::~ThreadMessage() = default;

ThreadMessage ThreadMessage::edit_text(const String& new_text) const {
    return ThreadMessage(
        this->message_service->edit_text(this->timetoken(), *this->data, new_text),
        this->parent_channel_id_internal
    );
}

ThreadMessage ThreadMessage::delete_message() const {
    return ThreadMessage(
        this->message_service->delete_message(*this->data, this->timetoken()),
        this->parent_channel_id_internal
    );
}

ThreadMessage ThreadMessage::restore() const {
    return ThreadMessage(
        this->message_service->restore(*this->data, this->timetoken()),
        this->parent_channel_id_internal
    );
}

ThreadMessage ThreadMessage::toggle_reaction(const String& reaction) const {
    return ThreadMessage(
        this->message_service->toggle_reaction(this->timetoken(), *this->data, reaction),
        this->parent_channel_id_internal
    );
}

//CallbackStop
//    ThreadMessage::stream_updates(std::function<void(const ThreadMessage&)> message_callback
//    ) const {
//    return CallbackStop(this->message_service->stream_updates(*this, message_callback));
//}

CallbackStop ThreadMessage::stream_updates_on(
    Pubnub::Vector<Pubnub::ThreadMessage> messages,
    std::function<void(Pubnub::Vector<Pubnub::ThreadMessage>)> message_callback
) const {
    auto messages_std = messages.into_std_vector();

    auto new_callback = [=](std::vector<Pubnub::ThreadMessage> vec) {
        message_callback(std::move(vec));
    };
    return CallbackStop(this->message_service->stream_updates_on(*this, messages_std, new_callback)
    );
}

Pubnub::Option<Pubnub::ThreadMessage> Pubnub::ThreadMessage::quoted_message() const {
    QuotedMessage quoted_message = this->data->to_entity().get_quoted_message();

    if (quoted_message.timetoken.empty()) {
        return Pubnub::Option<Pubnub::ThreadMessage>();
    }

    try {
        auto channel = this->channel_service->get_channel(message_data().channel_id);
        Pubnub::Message final_quoted_message = channel.get_message(quoted_message.timetoken);
        return ThreadMessage(final_quoted_message, this->parent_channel_id_internal);

    } catch (...) {
        return Pubnub::Option<Pubnub::ThreadMessage>();
    }

    return Pubnub::Option<Pubnub::ThreadMessage>();
}

Pubnub::Channel Pubnub::ThreadMessage::pin_to_parent_channel() const {
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);

    auto message_to_pin = this->to_message();

    return parent_channel.pin_message(message_to_pin);
}

Pubnub::Channel Pubnub::ThreadMessage::unpin_from_parent_channel() const {
    auto parent_channel = this->channel_service->get_channel(parent_channel_id_internal);
    return parent_channel.unpin_message();
}

Pubnub::Message Pubnub::ThreadMessage::to_message() const {
    return Message(
        this->timetoken(),
        this->chat_service,
        this->message_service,
        this->channel_service,
        this->restrictions_service,
        std::make_unique<MessageDAO>(this->data->to_entity())
    );
}

Pubnub::ThreadChannel Pubnub::ThreadMessage::create_thread() const {
    return this->channel_service->create_thread_channel(this->to_message());
}

Pubnub::ThreadChannel Pubnub::ThreadMessage::get_thread() const {
    return this->channel_service->get_thread_channel(this->to_message());
}

#ifdef PN_CHAT_C_ABI
ThreadMessage ThreadMessage::update_with_thread_base(const Pubnub::ThreadMessage& base_message
) const {
    return this->message_service->update_thread_message_with_base(*this, base_message);
}
#endif
