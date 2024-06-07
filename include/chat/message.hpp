#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include "enums.hpp"
#include "string.hpp"
#include "export.hpp"

namespace Pubnub
{
    class Chat;

    struct ChatMessageData
    {
        pubnub_chat_message_type type;
        Pubnub::String text;
        Pubnub::String channel_id;
        //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String user_id;
        //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String meta;
        std::vector<Pubnub::String> message_actions;
    };


    PN_CHAT_EXPORT class Message
    {
        public:
        PN_CHAT_EXPORT Message(Pubnub::Chat& in_chat, Pubnub::String in_timetoken, ChatMessageData in_message_data);
        PN_CHAT_EXPORT Message(Pubnub::Chat& in_chat, Pubnub::String in_channel_id, Pubnub::String in_message_data_json);
        ~Message() = default;

        PN_CHAT_EXPORT Message edit_text(Pubnub::String new_text);

        Pubnub::String to_string();

        PN_CHAT_EXPORT Pubnub::String get_timetoken();
        PN_CHAT_EXPORT ChatMessageData get_message_data();

        private:
        Pubnub::Chat& chat_obj;
        Pubnub::String timetoken;
        ChatMessageData message_data;
    };
}
#endif /* MESSAGE_H */
