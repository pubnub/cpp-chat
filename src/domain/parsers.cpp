#include "parsers.hpp"
#include "domain/channel_entity.hpp"
#include "enums.hpp"
#include "json.hpp"
#include <pubnub_helper.h>
#include <pubnub_memory_block.h>

bool Parsers::PubnubJson::is_message(Pubnub::String message_json_string)
{
    auto message_json = Json::parse(message_json_string);
    return message_json.contains("text") && message_json.contains("type");
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
    return Json::parse(string_from_pn_block(pn_block))
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

std::pair<Parsers::PubnubJson::ChannelId, ChannelEntity> Parsers::PubnubJson::to_channel(pubnub_v2_message pn_message) {
    auto json = Json::parse(string_from_pn_block(pn_message.payload));

    return std::make_pair(
            json_field_from_pn_block(pn_message.payload, "channel", "id"),
            ChannelEntity::from_json(json)
    );
}

std::pair<Parsers::PubnubJson::UserId, UserEntity> Parsers::PubnubJson::to_user(pubnub_v2_message pn_message) {
    auto json = Json::parse(string_from_pn_block(pn_message.payload));

    return std::make_pair(
            json_field_from_pn_block(pn_message.payload, "uuid", "id"),
            UserEntity::from_json(json)
    );
}

