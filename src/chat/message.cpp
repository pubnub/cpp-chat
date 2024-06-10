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

    if(message_data_json.contains("message"))
    {
        message_data.text = message_data_json["message"]["text"];
        message_data.type = chat_message_type_from_string(String(message_data_json["message"]["type"]));
    }
    if(message_data_json.contains("actions"))
    {
        json message_actions_json = message_data_json["actions"];
        //Add all message actions - for now as map of strings. We might need to create struct to hold message action data in more handy way.
        for (json::iterator message_action = message_actions_json.begin(); message_action != message_actions_json.end(); ++message_action) {
            message_data.message_actions[message_action.key()] = message_action.value();
        }
    }
}

Message Message::edit_text(Pubnub::String new_text)
{
    if(new_text.empty())
    {
        throw std::invalid_argument("Failed to edit text, new_text is empty");
    }

    pubnub_message_action_type edited_action_type = pubnub_message_action_type::PMAT_Edited;

    String action_timetoken = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, edited_action_type, new_text);
    message_data.text = new_text;
    add_message_action_to_message_data(action_timetoken, edited_action_type, new_text);
    return *this;
}

Pubnub::String Message::text()
{
    //TODO: this function has potential issue. If there are 2 objects for the same message and one is editted - another one will have
    //not actuall data. So can return incorrect text.

    //Check if there are any editions of current text
    if(message_data.message_actions.count(message_action_type_to_string(pubnub_message_action_type::PMAT_Edited)) > 0)
    {
        json message_actions_json = message_data.message_actions[message_action_type_to_string(pubnub_message_action_type::PMAT_Edited)];

        //Find the most recent edition by comparing timetokens
        String most_recent_edition;
        uint64_t most_recent_timetoken = -1;
        for (json::iterator message_action = message_actions_json.begin(); message_action != message_actions_json.end(); ++message_action) {
            message_data.message_actions[message_action.key()] = message_action.value();
            if(std::stoull(String(message_action.value()["timetoken"])) > most_recent_timetoken)
            {
                most_recent_timetoken = std::stoull(String(message_action.value()["timetoken"]));
                most_recent_edition = message_action.key();
            }
        }
        return most_recent_edition;
    }
    
    //There are no any editions, just return current text from data
    return message_data.text;
}

Message Message::delete_message()
{
    pubnub_message_action_type deleted_action_type = pubnub_message_action_type::PMAT_Deleted;
    String deleted_value = "\"deleted\"";
    String action_timetoken = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, pubnub_message_action_type::PMAT_Deleted, deleted_value);

    add_message_action_to_message_data(action_timetoken, deleted_action_type, deleted_value);
    return *this;
}

bool Message::deleted()
{
    //check if there is any message action of type "deleted"
    return message_data.message_actions.count(message_action_type_to_string(pubnub_message_action_type::PMAT_Deleted)) > 0;
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

void Message::add_message_action_to_message_data(Pubnub::String action_timetoken, pubnub_message_action_type action_type, Pubnub::String value)
{
    Pubnub::String existing_actions = message_data.message_actions[message_action_type_to_string(action_type)];

    json message_actions_json = json::object();

    if(!existing_actions.empty())
    {
        message_actions_json = json::parse(existing_actions);
    }

    //server response structure requires it to be an array
    json current_action_json_array = json::array();
    json current_action_json = json::object();
    current_action_json["uuid"] = message_data.user_id;
    current_action_json["actionTimetoken"] = action_timetoken;
    current_action_json_array.push_back(current_action_json);
    message_actions_json[value] = current_action_json_array;
}