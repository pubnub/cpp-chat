#include "infra/serialization.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;


bool Deserialization::is_chat_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("text") && message_json.contains("type");
}

bool Deserialization::is_message_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("data") && message_json["source"] == "actions";
}

bool Deserialization::is_channel_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "channel" && message_json["event"] == "set";
}

bool Deserialization::is_user_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "uuid" && message_json["event"] == "set";
}

bool Deserialization::is_event_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("event");
}

bool Deserialization::is_presence_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("action") && message_json.contains("uuid");
}

bool Deserialization::is_membership_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    //TODO: to finish
    return false;
}

Pubnub::Message Deserialization::pubnub_to_chat_message(Pubnub::Chat& chat_obj, pubnub_v2_message pn_message)
{
    // TODO: implement message parsing properly
    auto to_pn_string = [](struct pubnub_char_mem_block message) {
        return Pubnub::String(message.ptr, message.size);
    };

    json message_json = json::parse(to_pn_string(pn_message.payload));

    return Pubnub::Message(
            chat_obj,
            to_pn_string(pn_message.tt),
            Pubnub::ChatMessageData{
                Pubnub::pubnub_chat_message_type::PCMT_TEXT,
                Pubnub::String(message_json["text"]),
                to_pn_string(pn_message.channel),
                to_pn_string(pn_message.publisher),
                to_pn_string(pn_message.metadata),
                {}
            }
        );
}

Pubnub::Channel Deserialization::pubnub_message_to_chat_channel(Pubnub::Chat& chat_obj, pubnub_v2_message pn_message)
{
    // TODO: implement message parsing properly
    auto to_pn_string = [](struct pubnub_char_mem_block message) {
        return Pubnub::String(message.ptr, message.size);
    };

    json message_json = json::parse(to_pn_string(pn_message.payload));
    json channel_data_json = message_json["data"];

    return Pubnub::Channel(
            chat_obj,
            Pubnub::String(channel_data_json["id"]),
            Pubnub::ChatChannelData{
                Pubnub::String(channel_data_json["name"]),
                Pubnub::String(channel_data_json["description"]),
                Pubnub::String(channel_data_json["custom"]),
                Pubnub::String(channel_data_json["updated"]),
                Pubnub::String(channel_data_json["status"]),
                Pubnub::String(channel_data_json["type"])
            }
        );
}

Pubnub::User Deserialization::pubnub_message_to_chat_user(Pubnub::Chat& chat_obj, pubnub_v2_message pn_message)
{
    // TODO: implement message parsing properly
    auto to_pn_string = [](struct pubnub_char_mem_block message) {
        return Pubnub::String(message.ptr, message.size);
    };

    json message_json = json::parse(to_pn_string(pn_message.payload));
    json user_data_json = message_json["data"];

    return Pubnub::User(
            chat_obj,
            Pubnub::String(user_data_json["id"]),
            Pubnub::ChatUserData{
                Pubnub::String(user_data_json["name"]),
                Pubnub::String(user_data_json["externalId"]),
                Pubnub::String(user_data_json["profileUrl"]),
                Pubnub::String(user_data_json["email"]),
                Pubnub::String(user_data_json["custom"]),
                Pubnub::String(user_data_json["status"]),
                Pubnub::String(user_data_json["type"])
            }
        );
}

Pubnub::String Deserialization::pubnub_message_to_string(pubnub_v2_message pn_message)
{
    return Pubnub::String(pn_message.payload.ptr, pn_message.payload.size);
}
