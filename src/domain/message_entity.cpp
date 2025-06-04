#include "message_entity.hpp"
#include "domain/json.hpp"
#include "application/enum_converters.hpp"
#include "message_action.hpp"
#include <algorithm>
#include <optional>
#include <variant>
#include <iostream>

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
    new_message_entity.user_id = message_data_json.get_string("uuid").value_or(Pubnub::String(""));
    if(message_data_json.contains("meta") && !message_data_json["meta"].is_null())
    {
        Json meta_json = message_data_json["meta"];
        new_message_entity.meta = meta_json.dump();
    }
    else 
    {
        new_message_entity.meta = "";
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

    Json messages_array_json = history_json["channels"];

    if (!messages_array_json.contains(channel_id))
    {
        return messages;
    }

    for (auto element : messages_array_json[channel_id])
    {
        //Make sure this is actually a chat message as get history also return technical messages
        if(element.contains("message") && !element["message"].is_null())
        {
            Json message_json = element["message"];
            //TODO:: use enum instead of hardcoded type "text"
            if(message_json.contains("type") && message_json["type"] == "text")
            {
                auto new_message_entity = MessageEntity::from_json(element.dump(), channel_id);
                messages.push_back(std::make_pair(element["timetoken"], new_message_entity));
            }
        }

    }

    return messages;
}

MessageEntity MessageEntity::edit_text(const Pubnub::String& text, const Pubnub::String& timetoken) const {
    MessageEntity new_message_entity = *this;

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

std::map<int, Pubnub::MentionedUser> MessageEntity::get_mentioned_users() const
{
    std::map<int, Pubnub::MentionedUser> mentioned_users;
    if(meta.empty())
    {
        return mentioned_users;
    }

    Json metadata_json = Json::parse(meta);
    
    if(metadata_json.contains("mentionedUsers") && !metadata_json["mentionedUsers"].is_null())
    {
        auto mentioned_users_json = metadata_json["mentionedUsers"];
        for (Json::Iterator mentioned_user = mentioned_users_json.begin(); mentioned_user != mentioned_users_json.end(); ++mentioned_user) 
        {
            Json mentioned_object_json = mentioned_user.value();
            Pubnub::MentionedUser mentioned_user_data;
            int key = std::stoi(mentioned_user.key());
            mentioned_user_data.id = mentioned_object_json.get_string("id").value_or(Pubnub::String(""));
            mentioned_user_data.name = mentioned_object_json.get_string("name").value_or(Pubnub::String(""));

            mentioned_users[key] = mentioned_user_data;
        }
    }

    return mentioned_users;
}

std::map<int, Pubnub::ReferencedChannel> MessageEntity::get_referenced_channels() const
{
    std::map<int, Pubnub::ReferencedChannel> referenced_channels;
    if(meta.empty())
    {
        return referenced_channels;
    }

    Json metadata_json = Json::parse(meta);
    
    if(metadata_json.contains("referencedChannels") && !metadata_json["referencedChannels"].is_null())
    {
        auto referenced_channels_json = metadata_json["referencedChannels"];
        for (Json::Iterator referenced_channel = referenced_channels_json.begin(); referenced_channel != referenced_channels_json.end(); ++referenced_channel) 
        {
            Json mentioned_object_json = referenced_channel.value();
            Pubnub::ReferencedChannel referenced_channel_data;
            int key = std::stoi(referenced_channel.key());
            referenced_channel_data.id = mentioned_object_json.get_string("id").value_or(Pubnub::String(""));
            referenced_channel_data.name = mentioned_object_json.get_string("name").value_or(Pubnub::String(""));

            referenced_channels[key] = referenced_channel_data;
        }
    }

    return referenced_channels;
}

Pubnub::QuotedMessage MessageEntity::get_quoted_message() const
{
    if(meta.empty())
    {
        return Pubnub::QuotedMessage();
    }

    Json metadata_json = Json::parse(meta);

    if(metadata_json.contains("quotedMessage") && !metadata_json["quotedMessage"].is_null())
    {
        Pubnub::QuotedMessage quoted_message;
        Json quoted_message_json = metadata_json["quotedMessage"];
        quoted_message.text = quoted_message_json.get_string("text").value_or(Pubnub::String(""));
        quoted_message.channel_id = quoted_message_json.get_string("channelId").value_or(Pubnub::String(""));
        quoted_message.user_id = quoted_message_json.get_string("userId").value_or(Pubnub::String(""));
        quoted_message.timetoken = quoted_message_json.get_string("timetoken").value_or(Pubnub::String(""));

        return quoted_message;
    }
    return Pubnub::QuotedMessage();
}

std::vector<Pubnub::TextLink> MessageEntity::get_text_links() const
{
    std::vector<Pubnub::TextLink> text_links;
    if(meta.empty())
    {
        return text_links;
    }

    Json metadata_json = Json::parse(meta);

    if(metadata_json.contains("textLinks") && !metadata_json["textLinks"].is_null())
    {
        auto text_links_json_array = metadata_json["textLinks"];
        for (Json::Iterator text_link = text_links_json_array.begin(); text_link != text_links_json_array.end(); ++text_link) 
        {
            Json text_link_json = text_link.value();
            Pubnub::TextLink text_link_data;
            text_link_data.start_index = text_link_json.get_int("start_index").value_or(0);
            text_link_data.end_index = text_link_json.get_int("end_index").value_or(0);
            text_link_data.link = text_link_json.get_string("link").value_or(Pubnub::String(""));
            text_links.push_back(text_link_data);
        }
    }
    return text_links;
}

MessageEntity MessageEntity::from_base_and_updated_message(MessageEntity base_message, MessageEntity updated_message)
{
    MessageEntity new_entity;
    new_entity.type = base_message.type;
    new_entity.text = base_message.text;
    new_entity.channel_id = base_message.channel_id;
    new_entity.user_id = base_message.user_id;
    new_entity.meta = base_message.meta;

    new_entity.message_actions.insert(new_entity.message_actions.end(), base_message.message_actions.begin(), base_message.message_actions.end());
    new_entity.message_actions.insert(new_entity.message_actions.end(), updated_message.message_actions.begin(), updated_message.message_actions.end());

    std::sort(
            new_entity.message_actions.begin(),
            new_entity.message_actions.end(),
            [](const Pubnub::MessageAction& a, const Pubnub::MessageAction& b) {
                return a.timetoken < b.timetoken;
            }
    );

    new_entity.message_actions
        .erase(std::unique(
                    new_entity.message_actions.begin(),
                    new_entity.message_actions.end(),
                    [](const Pubnub::MessageAction& a, const Pubnub::MessageAction& b) {
                        return a.timetoken == b.timetoken && a.type == b.type && a.user_id == b.user_id && a.value == b.value;
                    }
                ),
            new_entity.message_actions.end()
        );

    return new_entity;
}
