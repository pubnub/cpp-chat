#ifndef PN_CHAT_MESSAGE_H
#define PN_CHAT_MESSAGE_H

#include <functional>
#include <memory>
#include <vector>

#include "base_message.hpp"
#include "callback_stop.hpp"
#include "helpers/export.hpp"
#include "message_action.hpp"
#include "message_elements.hpp"
#include "option.hpp"
#include "string.hpp"
#include "vector.hpp"

class MessageService;
class ChatService;
class ChannelService;
class RestrictionsService;
class MessageDAO;

namespace Pubnub {
    class ThreadChannel;
    class ThreadMessage;
    struct MentionedUser;

    class Message final: public BaseMessage<Message> {
        public:
            PN_CHAT_EXPORT Message();
            PN_CHAT_EXPORT Message(const Message& other);
            PN_CHAT_EXPORT Message& operator=(const Message& other);
            PN_CHAT_EXPORT ~Message();

            PN_CHAT_EXPORT Message edit_text(const Pubnub::String& new_text) const override;
            PN_CHAT_EXPORT Message delete_message() const override;
            PN_CHAT_EXPORT Message restore() const override;
            PN_CHAT_EXPORT Message toggle_reaction(const Pubnub::String& reaction) const override;
            PN_CHAT_EXPORT CallbackStop
                stream_updates(std::function<void(const Message&)> message_callback) const override;
            PN_CHAT_EXPORT CallbackStop stream_updates_on(
                Pubnub::Vector<Message> messages,
                std::function<void(Pubnub::Vector<Message>)> message_callback
            ) const override;
            PN_CHAT_EXPORT Pubnub::Option<Message> quoted_message() const override;
            PN_CHAT_EXPORT Pubnub::ThreadChannel create_thread() const override;
            PN_CHAT_EXPORT Pubnub::ThreadChannel get_thread() const override;



        private:
            PN_CHAT_EXPORT Message(
                Pubnub::String timetoken,
                std::shared_ptr<const ChatService> chat_service,
                std::shared_ptr<const MessageService> message_service,
                std::shared_ptr<const ChannelService> channel_service,
                std::shared_ptr<const RestrictionsService> restrictions_service,
                std::unique_ptr<MessageDAO> data
            );

            friend class ::MessageService;
            friend class ThreadMessage;

#ifdef PN_CHAT_C_ABI
        public:
            Pubnub::Message update_with_base(const Pubnub::Message& base_message) const;
#endif
    };
};  // namespace Pubnub
#endif /* PN_CHAT_MESSAGE_H */
