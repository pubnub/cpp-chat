#include <iostream>
#include "chat/message.hpp"
#include "chat.hpp"
#include "nlohmann/json.hpp"
#include "string.hpp"


using namespace Pubnub;
using json = nlohmann::json;

Message::Message(Pubnub::Chat& in_chat, String in_timetoken, ChatMessageData in_message_data) :
    chat_obj(in_chat),
    timetoken(in_timetoken),
    message_data(in_message_data)
{}

Message::Message(Pubnub::Chat& in_chat, String in_channel_id, Pubnub::String in_message_data_json) :
    chat_obj(in_chat)
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

Message Message::edit_text(Pubnub::String new_text)
{
    if(new_text.empty())
    {
        throw std::invalid_argument("Failed to edit text, new_text is empty");
    }

    String action_data = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, pubnub_message_action_type::PMAT_Edited, new_text);
    message_data.text = new_text;
    message_data.message_actions.push_back(action_data);
    return *this;
}

Message Message::delete_message()
{
    String deleted_value = "\"deleted\"";
    String action_data = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, pubnub_message_action_type::PMAT_Deleted, deleted_value);
    message_data.message_actions.push_back(action_data);
    return *this;
}

String Message::to_string() {
    //TODO: full conversion from message to json string
    return message_data.text;
}

Pubnub::String Message::get_timetoken()
{
    return timetoken;
}

ChatMessageData Message::get_message_data()
{
    return message_data;
}