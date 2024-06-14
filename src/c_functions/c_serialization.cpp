#include "infra/serialization.hpp"
#include "c_functions/c_serialization.hpp"
#include "c_functions/c_errors.hpp"
#include "chat/channel.hpp"
#include "chat/message.hpp"
#include "chat/user.hpp"
#include "chat.hpp"
#include "nlohmann/json.hpp"
#include "string.hpp"
#include <pubnub_helper.h>

extern "C" {
    #include <pubnub_api_types.h>
    #include <pubnub_helper.h>
}


using json = nlohmann::json;

Pubnub::Message* pn_deserialize_message(Pubnub::Chat* chat, pubnub_v2_message* message) {
    if (!Deserialization::is_chat_message(Pubnub::String(message->payload.ptr, message->payload.size))) {
        pn_c_set_error_message("Message is not a chat message");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::Message(Deserialization::pubnub_to_chat_message(*chat, *message));
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
        Pubnub::String string(presence_json->channel.ptr, presence_json->channel.size);
        strcpy(result, string.c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

Pubnub::Message* pn_deserialize_message_update(Pubnub::Chat* chat, pubnub_v2_message* message_update) {
    if (!Deserialization::is_message_update_message(Pubnub::String(message_update->payload.ptr, message_update->payload.size))) {
        pn_c_set_error_message("Message is not a chat message update");

        return PN_C_ERROR_PTR;
    }

    try {
        json message_json = json::parse(Pubnub::String(message_update->payload.ptr, message_update->payload.size));

        if(message_json.is_null()) {
            throw std::runtime_error("Failed to parse message into json");
        }

        auto channel = Pubnub::String(message_update->channel.ptr, message_update->channel.size);
        auto timetoken = message_json["data"]["messageTimetoken"].dump();

        return new Pubnub::Message(chat->get_channel(channel).get_message(timetoken));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Membership* pn_deserialize_membership(Pubnub::Chat* chat, pubnub_v2_message* membership) {
    if (!Deserialization::is_membership_update_message(Pubnub::String(membership->payload.ptr, membership->payload.size))) {
        pn_c_set_error_message("Message is not a chat membership update");

        return PN_C_ERROR_PTR;
    }

    try {
        json message_json = json::parse(Pubnub::String(membership->payload.ptr, membership->payload.size));

        if (message_json.is_null()) {
            throw std::runtime_error("Failed to parse message into json");
        }

        auto membership_channel = message_json["data"]["messageTimetoken"]["id"].dump();

        // TODO: implement that properly
        return nullptr;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}



void pn_dispose_message(pubnub_v2_message* message) {
    delete message;
}
