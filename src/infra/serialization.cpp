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
        message_json["source"] == "objects" && message_json["type"] == "channel";
}

bool Deserialization::is_user_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "uuid";
}

bool Deserialization::is_event_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    if(!message_json.contains("type") || message_json["type"].is_null())
    {
        return false;
    }

    bool is_event_message = message_json["type"] == "typing" || message_json["type"] == "report" || message_json["type"] == "receipt" ||
        message_json["type"] == "mention" || message_json["type"] == "invite" || message_json["type"] == "custom" || message_json["type"] == "moderation";
    
    return is_event_message;
}

bool Deserialization::is_presence_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("action") && message_json.contains("uuid");
}

bool Deserialization::is_membership_update_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "membership";
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
    Pubnub::String channel_data_string = channel_data_json.dump();

    return Pubnub::Channel(
            chat_obj,
            channel_data_string
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
    Pubnub::String user_data_string = user_data_json.dump();

    return Pubnub::User(
            chat_obj,
            user_data_string
        );
}

Pubnub::String Deserialization::pubnub_message_to_string(pubnub_v2_message pn_message)
{
    return Pubnub::String(pn_message.payload.ptr, pn_message.payload.size);
}
