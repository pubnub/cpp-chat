#ifndef PN_CHAT_THREAD_MESSAGE_HPP
#define PN_CHAT_THREAD_MESSAGE_HPP

#include "base_message.hpp"
#include "callback_stop.hpp"
#include "helpers/export.hpp"
#include "message.hpp"
#include "vector.hpp"

class CallbackService;

namespace Pubnub {
    class Channel;

    class ThreadMessage final: public BaseMessage<ThreadMessage> {
        public:
            PN_CHAT_EXPORT ~ThreadMessage();
            PN_CHAT_EXPORT ThreadMessage& operator=(const ThreadMessage& other);

            PN_CHAT_EXPORT ThreadMessage edit_text(const Pubnub::String& new_text) const override;
            PN_CHAT_EXPORT ThreadMessage delete_message() const override;
            PN_CHAT_EXPORT ThreadMessage restore() const override;
            PN_CHAT_EXPORT ThreadMessage toggle_reaction(const Pubnub::String& reaction
            ) const override;
            PN_CHAT_EXPORT Pubnub::Option<ThreadMessage> quoted_message() const override;

            PN_CHAT_EXPORT CallbackStop stream_updates(
                std::function<void(const ThreadMessage&)> message_callback
            ) const override;
            PN_CHAT_EXPORT CallbackStop stream_updates_on(
                Pubnub::Vector<ThreadMessage> messages,
                std::function<void(Pubnub::Vector<ThreadMessage>)> message_callback
            ) const override;

            PN_CHAT_EXPORT Pubnub::ThreadChannel create_thread() const override;
            PN_CHAT_EXPORT Pubnub::ThreadChannel get_thread() const override;

            PN_CHAT_EXPORT Pubnub::String parent_channel_id() const {
                return parent_channel_id_internal;
            };

            PN_CHAT_EXPORT Pubnub::Channel pin_to_parent_channel() const;
            PN_CHAT_EXPORT Pubnub::Channel unpin_from_parent_channel() const;

            PN_CHAT_EXPORT Pubnub::Message to_message() const;

        private:
            PN_CHAT_EXPORT ThreadMessage(
                Pubnub::String timetoken,
                std::shared_ptr<ChatService> chat_service,
                std::shared_ptr<MessageService> message_service,
                std::shared_ptr<ChannelService> channel_service,
                std::shared_ptr<RestrictionsService> restrictions_service,
                std::unique_ptr<MessageDAO> data,
                Pubnub::String parent_channel_id
            );

            PN_CHAT_EXPORT
            ThreadMessage(Pubnub::Message base_message, Pubnub::String parent_channel_id);

            Pubnub::String parent_channel_id_internal;
            friend class ::MessageService;
            friend class ::CallbackService;
#ifdef PN_CHAT_C_ABI
        public:
            Pubnub::ThreadMessage update_with_thread_base(const Pubnub::ThreadMessage& base_message
            ) const;
#endif
    };
};  // namespace Pubnub

#endif  // PN_CHAT_THREAD_MESSAGE_HPP
