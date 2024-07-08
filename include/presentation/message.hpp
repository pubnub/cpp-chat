#ifndef PN_CHAT_MESSAGE_H
#define PN_CHAT_MESSAGE_H

#include "string.hpp"
#include "export.hpp"
#include "message_action.hpp"
#include <memory>
#include <vector>
#include <functional>

class MessageService;
class ChatService;
class ChannelService;

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
            PN_CHAT_EXPORT Pubnub::ChatMessageData message_data();

            PN_CHAT_EXPORT Pubnub::Message edit_text(Pubnub::String new_text);
            PN_CHAT_EXPORT Pubnub::String text();
            PN_CHAT_EXPORT Pubnub::Message delete_message();
            PN_CHAT_EXPORT bool deleted();
            PN_CHAT_EXPORT Pubnub::pubnub_chat_message_type type();

            PN_CHAT_EXPORT void pin();

            PN_CHAT_EXPORT void stream_updates(std::function<void(Message)> message_callback);
            PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::Message> messages, std::function<void(Message)> message_callback);

        private:
            PN_CHAT_EXPORT Message(Pubnub::String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<ChannelService> channel_service);
            Pubnub::String timetoken_internal;
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<MessageService> message_service;
            std::shared_ptr<ChannelService> channel_service;

        friend class ::MessageService;
    };
}
#endif /* PN_CHAT_MESSAGE_H */
