#ifndef PN_CHAT_SERIALIZATION_H
#define PN_CHAT_SERIALIZATION_H

#include "string.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "user.hpp"
#include <pubnub_helper.h>
#include <chat.hpp>

namespace Serialization {
}

namespace Deserialization {
    // bool is_chat_message(Pubnub::String message_json_string);
    // bool is_message_update_message(Pubnub::String message_json_string);
    // bool is_channel_update_message(Pubnub::String message_json_string);
    // bool is_user_update_message(Pubnub::String message_json_string);
    // bool is_event_message(Pubnub::String message_json_string);
    // bool is_presence_message(Pubnub::String message_json_string);
    // bool is_membership_update_message(Pubnub::String message_json_string);

    // // TODO: tbh is more conversion than deserialization
    // Pubnub::Message pubnub_to_chat_message(Pubnub::Chat& chat, pubnub_v2_message pn_message);
    // Pubnub::Channel pubnub_message_to_chat_channel(Pubnub::Chat& chat, pubnub_v2_message pn_message);
    // Pubnub::User pubnub_message_to_chat_user(Pubnub::Chat& chat, pubnub_v2_message pn_message);
Pubnub::String pubnub_message_to_string(pubnub_v2_message pn_message);
}

#endif // PN_CHAT_SERIALIZATION_H
