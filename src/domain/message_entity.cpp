#include "message_entity.hpp"
#include "domain/json.hpp"
#include "enums.hpp"
#include "message_action.hpp"
#include <algorithm>
#include <optional>
#include <variant>

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
        Json message_json = message_data_json["message"];
        new_message_entity.text = message_json.get_string("text").value_or(Pubnub::String(""));
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

std::vector<std::pair<MessageEntity::MessageTimetoken, MessageEntity>> MessageEntity::from_history_json(Json history_json, Pubnub::String channel_id) {
    std::vector<std::pair<MessageEntity::MessageTimetoken, MessageEntity>> messages;

    Json messages_array_json = history_json["channels"][channel_id];

    for (auto element : messages_array_json)
    {
        auto new_message_entity = MessageEntity::from_json(element.dump(), channel_id);

        messages.push_back(std::make_pair(element["timetoken"], new_message_entity));
    }

    return messages;
}

MessageEntity MessageEntity::edit_text(const Pubnub::String& text, const Pubnub::String& timetoken) const {
    MessageEntity new_message_entity = *this;
    new_message_entity.text = text;

    Pubnub::MessageAction message_action;
    message_action.type = Pubnub::pubnub_message_action_type::PMAT_Edited;
    message_action.value = text;
    message_action.timetoken = timetoken;
    message_action.user_id = this->user_id;

    new_message_entity.message_actions.push_back(message_action);

    return new_message_entity;

}

Pubnub::String MessageEntity::current_text() const {
    std::vector<Pubnub::MessageAction> filtered;
    
    std::copy_if(this->message_actions.begin(), this->message_actions.end(), std::back_inserter(filtered), [](const Pubnub::MessageAction& action) {
        return action.type == Pubnub::pubnub_message_action_type::PMAT_Edited;
    });

    if (filtered.empty()) {
        return this->text;
    }

    std::sort(filtered.begin(), filtered.end(), [](const Pubnub::MessageAction& a, const Pubnub::MessageAction& b) {
        return a.timetoken < b.timetoken;
    });

    return filtered.back().value;
}

MessageEntity MessageEntity::delete_message(const Pubnub::String& text, const Pubnub::String& timetoken) const {
    MessageEntity new_message_entity = *this;

    Pubnub::MessageAction message_action;
    message_action.type = Pubnub::pubnub_message_action_type::PMAT_Deleted;
    message_action.timetoken = timetoken;
    message_action.value = text;
    message_action.user_id = this->user_id;

    new_message_entity.message_actions.push_back(message_action);

    return new_message_entity;
}

bool MessageEntity::is_deleted() const {
    return std::any_of(this->message_actions.begin(), this->message_actions.end(), [](const Pubnub::MessageAction& action) {
        return action.type == Pubnub::pubnub_message_action_type::PMAT_Deleted;
    });
}

std::vector<Pubnub::MessageAction> MessageEntity::get_reactions() const {
    std::vector<Pubnub::MessageAction> filtered;
    
    std::copy_if(this->message_actions.begin(), this->message_actions.end(), std::back_inserter(filtered), [](const Pubnub::MessageAction& action) {
        return action.type == Pubnub::pubnub_message_action_type::PMAT_Reaction;
    });

    return filtered;
}

std::optional<MessageEntity::ActionTimetoken> MessageEntity::get_user_reaction_timetoken(const Pubnub::String& user_id, const Pubnub::String& reaction) const {
    auto reactions = this->get_reactions();

    auto found = std::find_if(reactions.begin(), reactions.end(), [user_id, reaction](const Pubnub::MessageAction& action) {
        return action.user_id == user_id && action.value == reaction;
    });

    return found != reactions.end() ? std::optional(found->timetoken) : std::nullopt;
}

MessageEntity MessageEntity::remove_user_reaction(const MessageEntity::ActionTimetoken& timetoken) const {
    MessageEntity new_message_entity = *this;

    // Unsafe: we assume that timetoken exists
    auto found = std::find_if(new_message_entity.message_actions.begin(), new_message_entity.message_actions.end(), [timetoken](const Pubnub::MessageAction& action) {
        return action.timetoken == timetoken;
    });

    new_message_entity.message_actions.erase(found);

    return new_message_entity;
}

MessageEntity MessageEntity::add_user_reaction(const Pubnub::String& user_id, const Pubnub::String& reaction, const MessageEntity::ActionTimetoken& timetoken) const {
    MessageEntity new_message_entity = *this;

    Pubnub::MessageAction message_action;
    message_action.type = Pubnub::pubnub_message_action_type::PMAT_Reaction;
    message_action.value = reaction;
    message_action.timetoken = timetoken;
    message_action.user_id = user_id;

    new_message_entity.message_actions.push_back(message_action);

    return new_message_entity;
}
