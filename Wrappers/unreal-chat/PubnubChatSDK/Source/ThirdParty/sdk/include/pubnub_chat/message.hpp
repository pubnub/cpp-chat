#ifndef PN_CHAT_MESSAGE_H
#define PN_CHAT_MESSAGE_H

#include "string.hpp"
#include "helpers/export.hpp"
#include "message_action.hpp"
#include "vector.hpp"
#include <memory>
#include <vector>
#include <functional>

class MessageService;
class ChatService;
class ChannelService;
class RestrictionsService;
class MessageDAO;


namespace Pubnub
{
    class ThreadChannel;

    struct ChatMessageData
    {
        pubnub_chat_message_type type;
        Pubnub::String text;
        Pubnub::String channel_id;
        //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String user_id;
        //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String meta;
        Pubnub::Vector<MessageAction> message_actions;
    };

    class Message
    {
        public:
            PN_CHAT_EXPORT Message(const Message& other);
            PN_CHAT_EXPORT Message& operator=(const Message& other);
            PN_CHAT_EXPORT ~Message();

            PN_CHAT_EXPORT Pubnub::String timetoken() const;
            PN_CHAT_EXPORT Pubnub::ChatMessageData message_data() const;

            PN_CHAT_EXPORT Pubnub::Message edit_text(const Pubnub::String& new_text) const;
            PN_CHAT_EXPORT Pubnub::String text() const;
            PN_CHAT_EXPORT Pubnub::Message delete_message() const;
            PN_CHAT_EXPORT bool deleted() const;
            PN_CHAT_EXPORT Pubnub::pubnub_chat_message_type type() const;

            PN_CHAT_EXPORT void pin() const;
            PN_CHAT_EXPORT void unpin() const;
            PN_CHAT_EXPORT Pubnub::Message toggle_reaction(const Pubnub::String& reaction) const;
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::MessageAction> reactions() const;
            PN_CHAT_EXPORT bool has_user_reaction(const Pubnub::String& reaction) const;
            PN_CHAT_EXPORT void forward(const Pubnub::String& channel_id) const;
            PN_CHAT_EXPORT void report(const Pubnub::String& reason) const;

            PN_CHAT_EXPORT void stream_updates(std::function<void(const Message&)> message_callback) const;
            PN_CHAT_EXPORT void stream_updates_on(Pubnub::Vector<Pubnub::Message> messages, std::function<void(const Message&)> message_callback) const;

            PN_CHAT_EXPORT Pubnub::ThreadChannel create_thread() const;
            PN_CHAT_EXPORT Pubnub::ThreadChannel get_thread() const;
            PN_CHAT_EXPORT bool has_thread() const;
            PN_CHAT_EXPORT void remove_thread() const;

        protected:
            PN_CHAT_EXPORT Message(
                    Pubnub::String timetoken,
                    std::shared_ptr<const ChatService> chat_service,
                    std::shared_ptr<const MessageService> message_service,
                    std::shared_ptr<const ChannelService> channel_service,
                    std::shared_ptr<const RestrictionsService> restrictions_service,
                    std::unique_ptr<MessageDAO> data
                );

            Pubnub::String timetoken_internal;
            std::unique_ptr<MessageDAO> data;
            std::shared_ptr<const ChatService> chat_service;
            std::shared_ptr<const MessageService> message_service;
            std::shared_ptr<const ChannelService> channel_service;
            std::shared_ptr<const RestrictionsService> restrictions_service;

        friend class ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_H */