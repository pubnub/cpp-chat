#include "message_entity.hpp"
#include "domain/json.hpp"

MessageEntity MessageEntity::from_json(Pubnub::String message_json, Pubnub::String channel_id) {
    auto message_data_json = Json::parse(message_json);;

    // TODO: remove checks from domain
    if(message_data_json.is_null())
    {
        throw std::runtime_error("Failed to construct message, message_json is invalid Json");
    }

    if(!message_data_json.contains("timetoken") && !message_data_json["timetoken"].is_null())
    {
        throw std::runtime_error("Failed to construct message, message_json doesn't have  timetoken field");
    }

    MessageEntity new_message_entity;

    new_message_entity.channel_id = channel_id;

    if(message_data_json.contains("message") && !message_data_json["message"].is_null())
    {
        new_message_entity.text = message_data_json["message"]["text"];
        new_message_entity.type = chat_message_type_from_string(message_data_json["message"]["type"].dump());
    }
    if(message_data_json.contains("actions") && !message_data_json["actions"].is_null())
    {
        auto message_action_types_json = message_data_json["actions"];
        for (Json::Iterator message_action_type = message_action_types_json.begin(); message_action_type != message_action_types_json.end(); ++message_action_type) 
        {
            Json message_actions_json = message_action_type.value();
            Pubnub::String message_actions_json_string = message_actions_json.dump();
            for (Json::Iterator message_action = message_actions_json.begin(); message_action != message_actions_json.end(); ++message_action) 
            {
                Json single_message_action_json = message_action.value();
                Pubnub::String single_message_action_json_string = single_message_action_json.dump();
                // TODO: leak...
                Pubnub::MessageAction message_action_data;
                message_action_data.type = message_action_type_from_string(message_action_type.key());
                message_action_data.value = message_action.key();
                message_action_data.timetoken = single_message_action_json[0]["actionTimetoken"];
                message_action_data.user_id = single_message_action_json[0]["uuid"];
                new_message_entity.message_actions.push_back(message_action_data);
            }
        }
    }

    return new_message_entity;
}
