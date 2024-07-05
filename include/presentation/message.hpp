#ifndef PN_CHAT_MESSAGE_H
#define PN_CHAT_MESSAGE_H

#include "string.hpp"
#include "export.hpp"
#include "message_action.hpp"
#include <memory>
#include <vector>

class MessageService;
class ChatService;

namespace Pubnub
{
    struct ChatMessageData
    {
        pubnub_chat_message_type type;
        Pubnub::String text;
        Pubnub::String channel_id;
        //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String user_id;
        //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String meta;
        std::vector<MessageAction> message_actions;
    };

    class Message
    {
        public:
            PN_CHAT_EXPORT inline Pubnub::String timetoken(){return timetoken_internal;};
            //PN_CHAT_EXPORT Pubnub::String message_data();

        private:
            PN_CHAT_EXPORT Message(Pubnub::String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service);
            Pubnub::String timetoken_internal;
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<MessageService> message_service;

        friend class ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_H */
