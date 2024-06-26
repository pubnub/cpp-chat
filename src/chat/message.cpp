#include "chat/message.hpp"
#include <iostream>
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

    if(!message_data_json.contains("timetoken") && !message_data_json["timetoken"].is_null())
    {
        throw std::runtime_error("Failed to construct message, in_message_data_json doesn't have  timetoken field");
    }

    timetoken = message_data_json["timetoken"].dump();
    timetoken.erase(0, 1);
    timetoken.erase(timetoken.length() - 1, 1);

    ChatMessageData message_data;
    message_data.channel_id = in_channel_id;

    if(message_data_json.contains("message") && !message_data_json["message"].is_null())
    {
        message_data.text = message_data_json["message"]["text"].dump();
        message_data.type = chat_message_type_from_string(message_data_json["message"]["type"].dump());
    }
    if(message_data_json.contains("actions") && !message_data_json["actions"].is_null())
    {
        json message_action_types_json = message_data_json["actions"];
        for (json::iterator message_action_type = message_action_types_json.begin(); message_action_type != message_action_types_json.end(); ++message_action_type) 
        {
            json message_actions_json = message_action_type.value();
            String message_actions_json_string = message_actions_json.dump();
            for (json::iterator message_action = message_actions_json.begin(); message_action != message_actions_json.end(); ++message_action) 
            {
                json single_message_action_json = message_action.value();
                String single_message_action_json_string = single_message_action_json.dump();
                MessageActionData message_action_data;
                message_action_data.type = message_action_type_from_string(message_action_type.key());
                message_action_data.value = message_action.key();
                message_action_data.timetoken = single_message_action_json[0]["actionTimetoken"].dump();
                message_action_data.user_id = single_message_action_json[0]["uuid"].dump();
                message_data.message_actions.push_back(message_action_data);
            }
        }
    }

    this->message_data = message_data;
}

Message Message::edit_text(Pubnub::String new_text)
{
    if(new_text.empty())
    {
        throw std::invalid_argument("Failed to edit text, new_text is empty");
    }

    pubnub_message_action_type edited_action_type = pubnub_message_action_type::PMAT_Edited;

    //Message action value sent to server has to be in quotation marks
    String new_text_with_quotations = "\"" + new_text + "\"";
    String action_timetoken = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, message_action_type_to_string(edited_action_type), new_text_with_quotations);
    
    //But we store message text without quotations marks
    message_data.text = new_text;
    MessageActionData edited_message_action;
    edited_message_action.type = edited_action_type;
    edited_message_action.value = new_text;
    edited_message_action.timetoken = action_timetoken;
    edited_message_action.user_id = message_data.user_id;
    add_message_action_to_message_data(edited_message_action);
    return *this;
}

Pubnub::String Message::text()
{
    //TODO: this function has potential issue. If there are 2 objects for the same message and one is editted - another one will have
    //not actuall data. So can return incorrect text.

    String most_recent_edition = message_data.text;
    uint64_t most_recent_timetoken = 1;
    for(auto message_action : message_data.message_actions)
    {
        //check if there is any message action of type "edited"
        if(message_action.type == pubnub_message_action_type::PMAT_Edited)
        {
            String current_timetoken_string = message_action.timetoken;
            current_timetoken_string.erase(0, 1);
            current_timetoken_string.erase(current_timetoken_string.length() - 1, 1);
            
            auto timetoken_value = std::stoull(current_timetoken_string.to_std_string());
            //check if edition token is newer
            if(timetoken_value > most_recent_timetoken)
            {
                most_recent_timetoken = timetoken_value;
                most_recent_edition = message_action.value;
            }
        }
    }
    return most_recent_edition;
}

Message Message::delete_message()
{
    pubnub_message_action_type deleted_action_type = pubnub_message_action_type::PMAT_Deleted;
    String deleted_value = "\"deleted\"";
    String action_timetoken = chat_obj.get_pubnub_context().add_message_action(message_data.channel_id, timetoken, message_action_type_to_string(deleted_action_type), deleted_value);

    MessageActionData deleted_message_action;
    deleted_message_action.type = deleted_action_type;
    deleted_message_action.value = deleted_value;
    deleted_message_action.timetoken = action_timetoken;
    deleted_message_action.user_id = message_data.user_id;
    add_message_action_to_message_data(deleted_message_action);
    return *this;
}

bool Message::deleted()
{
    for(auto message_action : message_data.message_actions)
    {
        //check if there is any message action of type "deleted"
        if(message_action.type == pubnub_message_action_type::PMAT_Deleted)
        {
            return true;
        }
    }
    return false;
}

void Pubnub::Message::stream_updates(std::function<void(Message)> message_callback)
{
    std::vector<Pubnub::Message> messages;
    messages.push_back(*this);
    stream_updates_on(messages, message_callback);
}
void Pubnub::Message::stream_updates_on(std::vector<Pubnub::Message> messages, std::function<void(Message)> message_callback)
{
    if(messages.empty())
    {
        throw std::invalid_argument("Cannot stream message updates on an empty list");
    }

    for(auto message : messages)
    {
        chat_obj.get_pubnub_context().register_message_update_callback(timetoken, message_data.channel_id, message_callback);
        chat_obj.get_pubnub_context().subscribe_to_channel(message_data.channel_id);
    }
}

void Message::pin()
{
    Pubnub::Channel channel = chat_obj.get_channel(message_data.channel_id);
    chat_obj.pin_message_to_channel(*this, channel);
}

String Message::to_string()
{
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

void Message::add_message_action_to_message_data(Pubnub::MessageActionData message_action)
{
    message_data.message_actions.push_back(message_action);
}
