#include "infra/serialization.hpp"
#include "c_functions/c_serialization.hpp"
#include "c_functions/c_errors.hpp"
#include "chat/channel.hpp"
#include "chat/message.hpp"
#include "chat/user.hpp"
#include <pubnub_helper.h>

Pubnub::Message* pn_deserialize_message(Pubnub::Chat* chat, pubnub_v2_message* message_json) {
    if (!Deserialization::is_chat_message(Pubnub::String(message_json->payload.ptr, message_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat message");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::Message(Deserialization::pubnub_to_chat_message(*chat, *message_json));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_deserialize_channel(Pubnub::Chat* chat, pubnub_v2_message* channel_json) {
    if (!Deserialization::is_channel_update_message(Pubnub::String(channel_json->payload.ptr, channel_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat channel");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::Channel(Deserialization::pubnub_message_to_chat_channel(*chat, *channel_json));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::User* pn_deserialize_user(Pubnub::Chat* chat, pubnub_v2_message* user_json) {
    if (!Deserialization::is_user_update_message(Pubnub::String(user_json->payload.ptr, user_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat user");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::User(Deserialization::pubnub_message_to_chat_user(*chat, *user_json));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_deserialize_event(pubnub_v2_message* event_json, char* result) {
    if (!Deserialization::is_event_message(Pubnub::String(event_json->payload.ptr, event_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat event");

        return PN_C_ERROR;
    }

    try {
        auto string = Deserialization::pubnub_message_to_string(*event_json);
        strcpy(result, string.c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_deserialize_presence(pubnub_v2_message* presence_json, char* result) {
    if (!Deserialization::is_presence_message(Pubnub::String(presence_json->payload.ptr, presence_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat presence");

        return PN_C_ERROR;
    }

    try {
        auto string = Deserialization::pubnub_message_to_string(*presence_json);
        strcpy(result, string.c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_deserialize_message_update(pubnub_v2_message* message_update_json, char* result) {
    if (!Deserialization::is_message_update_message(Pubnub::String(message_update_json->payload.ptr, message_update_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat message update");

        return PN_C_ERROR;
    }

    try {
        auto string = Deserialization::pubnub_message_to_string(*message_update_json);
        strcpy(result, string.c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

void pn_dispose_message(Pubnub::Message* message) {
    delete message;
}
