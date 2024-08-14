// TODO: this file is a 
#include "const_values.hpp"
#include "domain/parsers.hpp"
#include "membership.hpp"
#include "infra/serialization.hpp"
#include "c_serialization.hpp"
#include "c_errors.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "user.hpp"
#include "chat.hpp"
#include "nlohmann/json.hpp"
#include "string.hpp"
#include <iostream>
#include <pubnub_helper.h>
#include "application/chat_service.hpp"
#include "application/message_service.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "application/membership_service.hpp"

extern "C" {
    #include <pubnub_api_types.h>
    #include <pubnub_helper.h>
}

using json = nlohmann::json;

Pubnub::Message* pn_deserialize_message(Pubnub::Chat* chat, pubnub_v2_message* message) {
    if (!Parsers::PubnubJson::is_message(Pubnub::String(message->payload.ptr, message->payload.size))) {
        pn_c_set_error_message("Message is not a chat message");

        return PN_C_ERROR_PTR;
    }

    try {
        auto parsed_message = Parsers::PubnubJson::to_message(*message);
        return new Pubnub::Message(chat->get_chat_service()->message_service->create_message_object(parsed_message));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::ThreadMessage* pn_deserialize_thread_message(Pubnub::Chat* chat, pubnub_v2_message* message) {
    if (!Parsers::PubnubJson::is_message(Pubnub::String(message->payload.ptr, message->payload.size))) {
        pn_c_set_error_message("Message is not a chat message");

        return PN_C_ERROR_PTR;
    }

    auto prefix_length = std::strlen(Pubnub::MESSAGE_THREAD_ID_PREFIX);

    if (message->channel.size < prefix_length) {
        pn_c_set_error_message("Message is not a chat thread message");

        return PN_C_ERROR_PTR;
    }

    if (std::strncmp(
                message->channel.ptr,
                Pubnub::MESSAGE_THREAD_ID_PREFIX,
                prefix_length
        ) != 0) {
        pn_c_set_error_message("Message is not a chat thread message");

        return PN_C_ERROR_PTR;
    }

    const auto timetoken_length = 17;
    const auto underscores = 2;
    const auto channel_padding_length = timetoken_length + prefix_length + underscores;

    auto channel = Pubnub::String(message->channel.ptr, message->channel.size);
    auto parent_channel = channel.substring(prefix_length, channel.length() - channel_padding_length);

    try {
        auto parsed_message = Parsers::PubnubJson::to_message(*message);
        return new Pubnub::ThreadMessage(chat->get_chat_service()->message_service->create_thread_message_object(
                    Parsers::PubnubJson::to_message(*message), parent_channel));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_deserialize_channel(Pubnub::Chat* chat, pubnub_v2_message* channel_json) {
    if (!Parsers::PubnubJson::is_channel_update(Pubnub::String(channel_json->payload.ptr, channel_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat channel");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::Channel(chat->get_chat_service()->channel_service->create_channel_object(Parsers::PubnubJson::to_channel(*channel_json)));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::User* pn_deserialize_user(Pubnub::Chat* chat, pubnub_v2_message* user_json) {
    if (!Parsers::PubnubJson::is_user_update(Pubnub::String(user_json->payload.ptr, user_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat user");

        return PN_C_ERROR_PTR;
    }

    try {
        return new Pubnub::User(chat->get_chat_service()->user_service->create_user_object(Parsers::PubnubJson::to_user(*user_json)));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_deserialize_event(pubnub_v2_message* event_json, char* result) {
    if (!Parsers::PubnubJson::is_event(Pubnub::String(event_json->payload.ptr, event_json->payload.size))) {
        pn_c_set_error_message("Message is not a chat event");

        return PN_C_ERROR;
    }

    try {
        //TODO: possibly redundant to go pubnub_v2_message -> event -> json
        auto event = Parsers::PubnubJson::to_event(*event_json);
        auto j = nlohmann::json{
            {"timeToken", event.timetoken.c_str()},
            {"type", chat_event_type_to_string(event.type).c_str()},
            {"userId", event.user_id.c_str()},
            {"channelId", event.channel_id.c_str()},
            {"payload", event.payload.c_str()},
        };
        strcpy(result, j.dump().c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_deserialize_presence(pubnub_v2_message* presence_json, char* result) {
    if (!Parsers::PubnubJson::is_presence(Pubnub::String(presence_json->payload.ptr, presence_json->payload.size))) {
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
    if (!Parsers::PubnubJson::is_message_update(Pubnub::String(message_update->payload.ptr, message_update->payload.size))) {
        pn_c_set_error_message("Message is not a chat message update");

        return PN_C_ERROR_PTR;
    }

    try {
        json message_json = json::parse(Pubnub::String(message_update->payload.ptr, message_update->payload.size));

        if(message_json.is_null()) {
            throw std::runtime_error("Failed to parse message into json");
        }

        return new Pubnub::Message(chat->get_chat_service()->message_service->create_message_object(
                    Parsers::PubnubJson::to_message_update(*message_update)));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::ThreadMessage* pn_deserialize_thread_message_update(Pubnub::Chat* chat, pubnub_v2_message* message_update) {
    if (!Parsers::PubnubJson::is_message_update(Pubnub::String(message_update->payload.ptr, message_update->payload.size))) {
        pn_c_set_error_message("Message is not a chat thread message update");

        return PN_C_ERROR_PTR;
    }

    if (std::strncmp(
                message_update->channel.ptr,
                Pubnub::MESSAGE_THREAD_ID_PREFIX,
                std::strlen(Pubnub::MESSAGE_THREAD_ID_PREFIX) - 1
        ) != 0) {
        pn_c_set_error_message("Message is not a chat thread message update");

        return PN_C_ERROR_PTR;
    }

    try {
        json message_json = json::parse(Pubnub::String(message_update->payload.ptr, message_update->payload.size));

        if(message_json.is_null()) {
            throw std::runtime_error("Failed to parse message into json");
        }

        return new Pubnub::ThreadMessage(chat->get_chat_service()->message_service->create_thread_message_object(
                    Parsers::PubnubJson::to_message_update(*message_update), Pubnub::String(message_update->channel.ptr, message_update->channel.size)));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}


Pubnub::Membership* pn_deserialize_membership(Pubnub::Chat* chat, pubnub_v2_message* membership) {
    if (!Parsers::PubnubJson::is_membership_update(Pubnub::String(membership->payload.ptr, membership->payload.size))) {
        pn_c_set_error_message("Message is not a chat membership update");

        return PN_C_ERROR_PTR;
    }

    try {
        json message_json = json::parse(Pubnub::String(membership->payload.ptr, membership->payload.size));

        if (message_json.is_null()) {
            throw std::runtime_error("Failed to parse message into json");
        }

        // TODO: dump ... too many allocations...
        Pubnub::String channel_string = message_json["data"]["channel"]["id"].dump();
        Pubnub::String channel_string_cleaned = Pubnub::String(&channel_string[1], channel_string.length() - 2);
        Pubnub::String user_string = message_json["data"]["uuid"]["id"].dump();
        Pubnub::String user_string_cleaned = Pubnub::String(&user_string[1], user_string.length() - 2);
        Pubnub::String membership_string = message_json["data"]["custom"].dump();

        // TODO: I don't know why but there is a racing condition here
        // if the strings are not kept alive - the ccore cannot find the user/channel
        auto channel_obj = chat->get_channel(channel_string_cleaned);
        auto user_obj = chat->get_user(user_string_cleaned);
        auto membership_service = chat->get_chat_service()->membership_service;
        auto user_service = chat->get_chat_service()->user_service;
        auto channel_service = chat->get_chat_service()->channel_service;

        return new Pubnub::Membership(chat->get_chat_service()->membership_service->create_membership_object(user_service->get_user(user_string_cleaned), channel_service->get_channel(channel_string_cleaned), Parsers::PubnubJson::membership_from_string(message_json.dump())));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

void pn_dispose_message(pubnub_v2_message* message) {
    delete message;
}
