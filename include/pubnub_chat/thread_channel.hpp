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
            PN_CHAT_EXPORT ThreadChannel(Pubnub::String channel_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<ChannelService> channel_service, std::shared_ptr<PresenceService> presence_service, 
                                   std::shared_ptr<RestrictionsService> restrictions_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<MembershipService> membership_service,
                                   Pubnub::String parent_channel_id, Pubnub::Message parent_message);

            Pubnub::String parent_channel_id;
            Pubnub::Message parent_message;

      
    };
};

#endif // PN_CHAT_THREAD_CHANNEL_HPP
