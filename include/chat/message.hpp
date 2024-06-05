#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include "string.hpp"
#include "export.hpp"

namespace Pubnub
{
    enum pubnub_chat_message_type : uint8_t
    {
        PCMT_TEXT
    };

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
        PN_CHAT_EXPORT Message(Pubnub::String in_timetoken, ChatMessageData in_message_data);
        PN_CHAT_EXPORT Message(Pubnub::String in_timetoken, Pubnub::String in_message_data_json);
        ~Message() = default;

        private:

        Pubnub::String timetoken;
        ChatMessageData message_data;
    };
}
#endif /* MESSAGE_H */
