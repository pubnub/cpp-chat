#ifndef PN_CHAT_THREAD_MESSAGE_HPP
#define PN_CHAT_THREAD_MESSAGE_HPP

#include "pubnub_chat/message.hpp"


namespace Pubnub 
{
    class ThreadMessage : public Message
    {
        public:

        private:
            PN_CHAT_EXPORT ThreadMessage(Pubnub::String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<ChannelService> channel_service,
                                    std::shared_ptr<RestrictionsService> restrictions_service, std::unique_ptr<MessageDAO> data, Pubnub::String parent_channel_id);

            Pubnub::String parent_channel_id;

            PN_CHAT_EXPORT ThreadMessage& operator=(const ThreadMessage& other);
      
    };
};

#endif // PN_CHAT_THREAD_MESSAGE_HPP
