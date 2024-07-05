#include "parsers.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool Parsers::PubnubJson::is_message(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("text") && message_json.contains("type");
}

bool Parsers::PubnubJson::is_message_update(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("data") && message_json["source"] == "actions";
}

bool Parsers::PubnubJson::is_channel_update(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "channel";
}

bool Parsers::PubnubJson::is_user_update(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "uuid";
}

bool Parsers::PubnubJson::is_event(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    if(!message_json.contains("type") || message_json["type"].is_null())
    {
        return false;
    }

    //TODO: this could be done much more beautiful :) 
    bool is_event_message = message_json["type"] == "typing" || message_json["type"] == "report" || message_json["type"] == "receipt" ||
        message_json["type"] == "mention" || message_json["type"] == "invite" || message_json["type"] == "custom" || message_json["type"] == "moderation";

    return is_event_message;
}

bool Parsers::PubnubJson::is_presence(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("action") && message_json.contains("uuid");
}

bool Parsers::PubnubJson::is_membership_update(Pubnub::String message_json_string)
{
    json message_json = json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "membership";
}


