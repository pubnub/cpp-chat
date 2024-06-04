#include <iostream>
#include "chat/message.hpp"
#include "nlohmann/json.hpp"


using namespace Pubnub;
using json = nlohmann::json;

Message::Message(String in_timetoken, ChatMessageData in_message_data)
{
    timetoken = in_timetoken;
    message_data = in_message_data;
}

Message::Message(String in_channel_id, Pubnub::String in_message_data_json)
{
    json message_data_json = json::parse(in_message_data_json);;

    if(message_data_json.is_null())
    {
        throw std::runtime_error("Failed to construct message, in_message_data_json is invalid Json");
    }

    if(!message_data_json.contains("timetoken"))
    {
        throw std::runtime_error("Failed to construct message, in_message_data_json doesn't have  timetoken field");
    }

    timetoken = message_data_json["timetoken"];

    ChatMessageData message_data;
    message_data.channel_id = in_channel_id;

    if(message_data_json.contains("message") )
    {
        message_data.text = message_data_json["message"];
    }
    if(message_data_json.contains("actions") )
    {
        json message_actions_array_json = message_data_json["actions"];
        //Add all message actions - for now as strings. We might need to create struct to hold message action data in more handy way.
        for (auto& element : message_actions_array_json)
        {
            message_data.message_actions.push_back(element.dump());
        }
    }
}

