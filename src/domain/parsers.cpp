#include "parsers.hpp"
#include "domain/channel_entity.hpp"
#include "enums.hpp"
#include "json.hpp"
#include "message_action.hpp"
#include <pubnub_helper.h>
#include <pubnub_memory_block.h>

bool Parsers::PubnubJson::is_message(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("text") && message_json.contains("type") && message_json["type"] == "text";
}

bool Parsers::PubnubJson::is_message_update(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("data") && message_json["source"] == "actions";
}

bool Parsers::PubnubJson::is_channel_update(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "channel";
}

bool Parsers::PubnubJson::is_user_update(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "uuid";
}

bool Parsers::PubnubJson::is_event(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
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
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("action") && message_json.contains("uuid");
}

bool Parsers::PubnubJson::is_membership_update(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("source") && message_json.contains("type") &&  message_json.contains("event") && 
        message_json["source"] == "objects" && message_json["type"] == "membership";
}

static Pubnub::String string_from_pn_block(struct pubnub_char_mem_block pn_block)
{
    return Pubnub::String(pn_block.ptr, pn_block.size);
}

static Pubnub::String json_field_from_pn_block(struct pubnub_char_mem_block pn_block, Pubnub::String field)
{
    return Json::parse(string_from_pn_block(pn_block))
        .get_string(field)
        .value_or(Pubnub::String());
}

static Pubnub::String json_field_from_pn_block(struct pubnub_char_mem_block pn_block, Pubnub::String field, Pubnub::String subfield)
{
    auto outer_block = string_from_pn_block(pn_block); 

    if (outer_block.empty())
    {
        return Pubnub::String();
    }

    auto parsed = Json::parse(outer_block);

    if (!parsed.contains(field))
    {
        return Pubnub::String();
    }

    return parsed 
        [field]
        .get_string(subfield)
        .value_or(Pubnub::String());
}

std::pair<Parsers::PubnubJson::Timetoken, MessageEntity> Parsers::PubnubJson::to_message(pubnub_v2_message pn_message)
{
    return std::make_pair(
        string_from_pn_block(pn_message.tt),
        MessageEntity{
            // TODO: leak of presentation
            Pubnub::pubnub_chat_message_type::PCMT_TEXT,
            json_field_from_pn_block(pn_message.payload, "text"),
            string_from_pn_block(pn_message.channel),
            string_from_pn_block(pn_message.publisher),
            string_from_pn_block(pn_message.metadata),
            {}
        }
    );
}

std::pair<Parsers::PubnubJson::Timetoken, MessageEntity> Parsers::PubnubJson::to_message_update(pubnub_v2_message pn_message)
{
    auto reaction_id_from_json = json_field_from_pn_block(pn_message.payload, "data", "uuid");
    std::vector<Pubnub::MessageAction> message_actions;

    // Message update contains always the last action if it was the update
    if (json_field_from_pn_block(pn_message.payload, "event") != Pubnub::String("removed")) {
        message_actions.push_back(Pubnub::MessageAction{
                Pubnub::message_action_type_from_string(json_field_from_pn_block(pn_message.payload, "data", "type")),
                json_field_from_pn_block(pn_message.payload, "data", "value"),
                json_field_from_pn_block(pn_message.payload, "data", "actionTimetoken"),
                reaction_id_from_json.empty() ? string_from_pn_block(pn_message.publisher) : reaction_id_from_json                
           });
    }

    return std::make_pair(
        json_field_from_pn_block(pn_message.payload, "data", "messageTimetoken"),
        MessageEntity{
            // TODO: leak of presentation
            Pubnub::pubnub_chat_message_type::PCMT_TEXT,
            // TODO: message should only care about message actions - consider leaving empty string
            json_field_from_pn_block(pn_message.payload, "data", "value"),
            string_from_pn_block(pn_message.channel),
            string_from_pn_block(pn_message.publisher),
            string_from_pn_block(pn_message.metadata),
            message_actions
            
        }
    );
}

std::pair<Parsers::PubnubJson::ChannelId, ChannelEntity> Parsers::PubnubJson::to_channel(pubnub_v2_message pn_message) {
    auto json = Json::parse(string_from_pn_block(pn_message.payload));

    return std::make_pair(
            json_field_from_pn_block(pn_message.payload, "data", "id"),
            ChannelEntity::from_channel_response(json["data"])
    );
}

std::pair<Parsers::PubnubJson::UserId, UserEntity> Parsers::PubnubJson::to_user(pubnub_v2_message pn_message) {
    auto json = Json::parse(string_from_pn_block(pn_message.payload));

    return std::make_pair(
            json_field_from_pn_block(pn_message.payload, "data", "id"),
            UserEntity::from_user_response(json["data"])
    );
}

MembershipEntity Parsers::PubnubJson::membership_from_string(Pubnub::String message_json) {
    auto json = Json::parse(message_json);

    return MembershipEntity::from_json(json["data"]);
}

Pubnub::String Parsers::PubnubJson::to_string(pubnub_v2_message pn_message) {
    return string_from_pn_block(pn_message.payload);
}

Pubnub::Event Parsers::PubnubJson::to_event(pubnub_v2_message pn_message)
{
    return Pubnub::Event({
        string_from_pn_block(pn_message.tt),
        Pubnub::chat_event_type_from_string(event_type(string_from_pn_block(pn_message.payload))),
        string_from_pn_block(pn_message.channel),
        string_from_pn_block(pn_message.publisher),
        string_from_pn_block(pn_message.payload)
    });
}

Pubnub::String Parsers::PubnubJson::event_type(Pubnub::String message_json) {
    return Json::parse(message_json)["type"];
}

Pubnub::String Parsers::PubnubJson::message_update_timetoken(Pubnub::String message_json) {
    return Json::parse(message_json)["data"]["messageTimetoken"];
}

Pubnub::String Parsers::PubnubJson::membership_channel(Pubnub::String message_json) {
    return Json::parse(message_json)["data"]["channel"]["id"];
}

Pubnub::String Parsers::PubnubJson::membership_user(Pubnub::String message_json) {
    return Json::parse(message_json)["data"]["uuid"]["id"];
}

Pubnub::String Parsers::PubnubJson::membership_custom_field(Pubnub::String message_json) {
    return Json::parse(message_json)["custom"];
}

bool Parsers::PubnubJson::contains_parent_message(Pubnub::String message_json_string) {
    if (message_json_string.empty())
    {
        return false;
    }

    auto parsed = Json::parse(message_json_string);

    if (parsed.is_null() || !parsed.contains("data"))
    {
        return false;
    }

    auto message_json = parsed["data"];

    if (message_json.is_null())
    {
        return false;
    }

    return message_json.contains("parentMessage");
}
