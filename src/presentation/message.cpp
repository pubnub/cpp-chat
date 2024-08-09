#include "message.hpp"
#include "application/message_service.hpp"
#include "application/channel_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/dao/message_dao.hpp"
#include "thread_channel.hpp"

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

bool Pubnub::Message::delete_message_hard() const
{
    //somehow it was returning false if wasn't passed to a variable
    bool deleted = this->message_service->delete_message_hard(*this);
    return deleted;
}

bool Message::deleted() const {
    return this->message_service->deleted(*this->data);
}

Message Message::restore() const
{
    return this->message_service->restore(*this->data, this->timetoken());
}

pubnub_chat_message_type Message::type() const {
    return this->message_data().type;
}

void Message::pin() const {
    Channel channel = this->channel_service->get_channel(message_data().channel_id);
    channel.pin_message(*this);
}

void Message::unpin() const {
    Channel channel = this->channel_service->get_channel(message_data().channel_id);

    //Make sure that this message is pinned to the channel
    bool is_this_message_pinned = false;
    try {
        auto pinned_message = channel.get_pinned_message();
        is_this_message_pinned = timetoken() == pinned_message.timetoken();
    } catch(const std::exception &e) {
        is_this_message_pinned = false;
    }

    //Unpin the message if that's the pinned message
    if(is_this_message_pinned)
    {
        channel.unpin_message();
    }
}

Message Message::toggle_reaction(const String& reaction) const {
    return this->message_service->toggle_reaction(this->timetoken(), *this->data, reaction);
}

Pubnub::Vector<MessageAction> Message::reactions() const {
    return Pubnub::Vector<MessageAction>(std::move(this->message_service->get_message_reactions(*this->data)));
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

CallbackStop Message::stream_updates(std::function<void(const Message&)> message_callback) const {
    return CallbackStop(this->message_service->stream_updates(*this, message_callback));
}

CallbackStop Message::stream_updates_on(Pubnub::Vector<Pubnub::Message> messages, std::function<void(Pubnub::Vector<Pubnub::Message>)> message_callback) const {
    auto messages_std = messages.into_std_vector();

    auto new_callback = [=](std::vector<Pubnub::Message> vec)
    {
        message_callback(std::move(vec));
    };
    return CallbackStop(this->message_service->stream_updates_on(*this, messages_std, new_callback));
}

ThreadChannel Message::create_thread() const
{
    return this->channel_service->create_thread_channel(*this);
}

ThreadChannel Message::get_thread() const
{
    return this->channel_service->get_thread_channel(*this);
}

bool Message::has_thread() const
{
    return this->channel_service->has_thread_channel(*this);
}

void Message::remove_thread() const
{
    this->channel_service->remove_thread_channel(*this);
}

Pubnub::Vector<MentionedUser> Message::mentioned_users() const
{
    auto users_map = this->data->to_entity().get_mentioned_users();
    std::vector<MentionedUser> users_vector;
    for(auto it = users_map.begin(); it != users_map.end(); it++)
    {
        users_vector.push_back(it->second);
    }
    return Pubnub::Vector<MentionedUser>(std::move(users_vector));
}

Pubnub::Vector<ReferencedChannel> Message::referenced_channels() const
{
    auto channels_map = this->data->to_entity().get_referenced_channels();
    std::vector<ReferencedChannel> channels_vector;
    for(auto it = channels_map.begin(); it != channels_map.end(); it++)
    {
        channels_vector.push_back(it->second);
    }
    return Pubnub::Vector<ReferencedChannel>(std::move(channels_vector));
}

Pubnub::Option<Pubnub::Message> Pubnub::Message::quoted_message() const
{
    QuotedMessage quoted_message = this->data->to_entity().get_quoted_message();
    if(quoted_message.timetoken.empty())
    {
        return Pubnub::Option<Pubnub::Message>();
    }

    try {
        auto channel = this->channel_service->get_channel(message_data().channel_id);
        Pubnub::Message final_quoted_message = channel.get_message(quoted_message.timetoken);
        return final_quoted_message;
        
    }
    catch (...)
    {
        return Pubnub::Option<Pubnub::Message>();
    }

    return Pubnub::Option<Pubnub::Message>();
}

Pubnub::Vector<TextLink> Message::text_links() const
{
    auto text_links = this->data->to_entity().get_text_links();
    return Pubnub::Vector<TextLink>(std::move(text_links));
}

#ifdef PN_CHAT_C_ABI
Pubnub::Message Pubnub::Message::update_with_base(const Pubnub::Message& base_message) const {
    return this->message_service->update_message_with_base(*this, base_message);
}
#endif
