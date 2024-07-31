#ifndef PN_CHAT_THREAD_MESSAGE_HPP
#define PN_CHAT_THREAD_MESSAGE_HPP

#include "pubnub_chat/message.hpp"


namespace Pubnub 
{
    class Channel;

    class ThreadMessage : public Message
    {
        public:
        PN_CHAT_EXPORT ~ThreadMessage();
        PN_CHAT_EXPORT ThreadMessage& operator=(const ThreadMessage& other);

        PN_CHAT_EXPORT Pubnub::String parent_channel_id() const {return parent_channel_id_internal;};

        PN_CHAT_EXPORT Pubnub::Channel pin_to_parent_channel() const;
        PN_CHAT_EXPORT Pubnub::Channel unpin_from_parent_channel() const;

        private:
            PN_CHAT_EXPORT ThreadMessage(
                    Pubnub::String timetoken, 
                    std::shared_ptr<ChatService> chat_service, 
                    std::shared_ptr<MessageService> message_service, 
                    std::shared_ptr<ChannelService> channel_service,
                    std::shared_ptr<RestrictionsService> restrictions_service, 
                    std::unique_ptr<MessageDAO> data, 
                    Pubnub::String parent_channel_id);

            PN_CHAT_EXPORT ThreadMessage(Pubnub::Message base_message, Pubnub::String parent_channel_id);

            Pubnub::String parent_channel_id_internal;
        friend class ::MessageService;
      
    };
};

#endif // PN_CHAT_THREAD_MESSAGE_HPP
