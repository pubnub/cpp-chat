#ifndef PN_CHAT_MESSAGE_ENTITY_HPP
#define PN_CHAT_MESSAGE_ENTITY_HPP

#include "enums.hpp"
#include "string.hpp"
#include <vector>

struct MessageEntity {
        struct MessageActionData
        {
            Pubnub::pubnub_message_action_type type;
            Pubnub::String value;
            Pubnub::String timetoken;
            Pubnub::String user_id;
        };

        Pubnub::pubnub_chat_message_type type;
        Pubnub::String text;
        Pubnub::String channel_id;
        //user_id doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String user_id;
        //meta doesn't work for now, because we don't get this info from pubnub_fetch_history
        Pubnub::String meta;
        std::vector<MessageActionData> message_actions;
};

#endif // PN_CHAT_MESSAGE_ENTITY_HPP
