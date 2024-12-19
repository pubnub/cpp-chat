#ifndef PN_CHAT_MESSAGE_DATA_HPP
#define PN_CHAT_MESSAGE_DATA_HPP

#include "enums.hpp"
#include "message_action.hpp"
#include "string.hpp"
#include "vector.hpp"

namespace Pubnub {
    struct ChatMessageData {
        pubnub_chat_message_type type;
        Pubnub::String text;
        Pubnub::String channel_id;
        //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String user_id;
        //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String meta;
        Pubnub::Vector<Pubnub::MessageAction> message_actions;
    };
}

#endif // PN_CHAT_MESSAGE_DATA_HPP
   
