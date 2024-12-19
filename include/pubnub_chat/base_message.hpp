#ifndef PN_CHAT_BASE_MESSAGE_HPP
#define PN_CHAT_BASE_MESSAGE_HPP

#include "callback_stop.hpp"
#include "chat_message_data.hpp"
#include "helpers/export.hpp"
#include "message_action.hpp"
#include "message_elements.hpp"
#include "string.hpp"
#include "vector.hpp"

// Forward declarations needed of services
class MessageService;
class ChatService;
class ChannelService;
class RestrictionsService;
class MessageDAO;

namespace Pubnub {
    class ThreadChannel;

    template<typename MessageType>
    class BaseMessage {
        public:
            PN_CHAT_EXPORT BaseMessage();
            PN_CHAT_EXPORT BaseMessage(const BaseMessage<MessageType>& other);

            PN_CHAT_EXPORT BaseMessage<MessageType>& operator=(const BaseMessage<MessageType>& other
            );

            PN_CHAT_EXPORT virtual ~BaseMessage();

            // Common behaviour

            PN_CHAT_EXPORT Pubnub::String timetoken() const;
            PN_CHAT_EXPORT Pubnub::ChatMessageData message_data() const;

            PN_CHAT_EXPORT Pubnub::String text() const;
            PN_CHAT_EXPORT bool delete_message_hard() const;
            PN_CHAT_EXPORT bool deleted() const;
            PN_CHAT_EXPORT Pubnub::pubnub_chat_message_type type() const;

            PN_CHAT_EXPORT void pin() const;
            PN_CHAT_EXPORT void unpin() const;
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::MessageAction> reactions() const;
            PN_CHAT_EXPORT bool has_user_reaction(const Pubnub::String& reaction) const;
            PN_CHAT_EXPORT void forward(const Pubnub::String& channel_id) const;
            PN_CHAT_EXPORT void report(const Pubnub::String& reason) const;

           PN_CHAT_EXPORT bool has_thread() const;
            PN_CHAT_EXPORT void remove_thread() const;

            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::MentionedUser> mentioned_users() const;
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::ReferencedChannel> referenced_channels() const;
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::TextLink> text_links() const;

            // Virtual methods

            PN_CHAT_EXPORT virtual MessageType edit_text(const Pubnub::String& new_text) const = 0;
            PN_CHAT_EXPORT virtual MessageType delete_message() const = 0;
            PN_CHAT_EXPORT virtual MessageType restore() const = 0;
            PN_CHAT_EXPORT virtual MessageType toggle_reaction(const Pubnub::String& reaction
            ) const = 0;
            PN_CHAT_EXPORT virtual Pubnub::Option<MessageType> quoted_message() const = 0;

            PN_CHAT_EXPORT virtual CallbackStop
                stream_updates(std::function<void(const MessageType&)> message_callback) const = 0;
            PN_CHAT_EXPORT virtual CallbackStop stream_updates_on(
                Pubnub::Vector<MessageType> messages,
                std::function<void(Pubnub::Vector<MessageType>)> message_callback
            ) const = 0;
            PN_CHAT_EXPORT virtual Pubnub::ThreadChannel create_thread() const = 0;
            PN_CHAT_EXPORT virtual Pubnub::ThreadChannel get_thread() const = 0;

        protected:
            PN_CHAT_EXPORT BaseMessage(
                String timetoken,
                std::shared_ptr<const ChatService> chat_service,
                std::shared_ptr<const MessageService> message_service,
                std::shared_ptr<const ChannelService> channel_service,
                std::shared_ptr<const RestrictionsService> restrictions_service,
                std::unique_ptr<MessageDAO> data
            );

            std::unique_ptr<MessageDAO> data;
            std::shared_ptr<const MessageService> message_service;
            std::shared_ptr<const ChannelService> channel_service;
            Pubnub::String timetoken_internal;
            std::shared_ptr<const ChatService> chat_service;
            std::shared_ptr<const RestrictionsService> restrictions_service;
    };
}  // namespace Pubnub





#endif  // PN_CHAT_BASE_MESSAGE_HPP
