#ifndef PN_CHAT_THREAD_CHANNEL_HPP
#define PN_CHAT_THREAD_CHANNEL_HPP

#include "pubnub_chat/channel.hpp"
#include "pubnub_chat/message.hpp"


namespace Pubnub 
{
    class ThreadChannel : public Channel
    {
        public:

        private:
            PN_CHAT_EXPORT ThreadChannel(
                    Pubnub::String channel_id, 
                    std::shared_ptr<const ChatService> chat_service, 
                    std::shared_ptr<const ChannelService> channel_service, 
                    std::shared_ptr<const PresenceService> presence_service, 
                    std::shared_ptr<const RestrictionsService> restrictions_service, 
                    std::shared_ptr<const MessageService> message_service, 
                    std::shared_ptr<const MembershipService> membership_service,
                    std::unique_ptr<ChannelDAO> data,
                    Pubnub::String parent_channel_id, 
                    Pubnub::Message parent_message);

            PN_CHAT_EXPORT ThreadChannel& operator=(const ThreadChannel& other);

            Pubnub::String parent_channel_id;
            Pubnub::Message parent_message;

        friend class ::ChannelService;

      
    };
};

#endif // PN_CHAT_THREAD_CHANNEL_HPP
