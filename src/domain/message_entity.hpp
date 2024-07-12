#ifndef PN_CHAT_MESSAGE_ENTITY_HPP
#define PN_CHAT_MESSAGE_ENTITY_HPP

#include "enums.hpp"
#include "string.hpp"
#include "presentation/message_action.hpp"
#include <vector>

struct MessageEntity {
    Pubnub::pubnub_chat_message_type type;
    Pubnub::String text;
    Pubnub::String channel_id;
    Pubnub::String user_id;
    Pubnub::String meta;
    std::vector<Pubnub::MessageAction> message_actions;

    static MessageEntity from_json(Pubnub::String message_json, Pubnub::String channel_id);
};

#endif // PN_CHAT_MESSAGE_ENTITY_HPP
