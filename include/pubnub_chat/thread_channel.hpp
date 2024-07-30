#ifndef PN_CHAT_THREAD_CHANNEL_HPP
#define PN_CHAT_THREAD_CHANNEL_HPP

#include "pubnub_chat/channel.hpp"
#include "pubnub_chat/message.hpp"


namespace Pubnub 
{
    class ThreadChannel : public Channel
    {
        public:

        PN_CHAT_EXPORT ~ThreadChannel();

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


            PN_CHAT_EXPORT void send_text(const Pubnub::String& message, Pubnub::pubnub_chat_message_type message_type = Pubnub::pubnub_chat_message_type::PCMT_TEXT, const Pubnub::String& meta_data = "") override;


            Pubnub::String parent_channel_id;
            Pubnub::Message parent_message;

        friend class ::ChannelService;



        private:
        //Bool to track if there was at least one message sent, because the thread is created on server during sending the first message
        bool is_thread_created = true;

        void set_is_thread_created(bool is_created);

      
    };
};

#endif // PN_CHAT_THREAD_CHANNEL_HPP
