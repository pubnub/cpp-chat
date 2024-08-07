#include "channel_entity.hpp"
#include "domain/json.hpp"

using json = nlohmann::json;

Pubnub::String ChannelEntity::get_channel_metadata_json_string(Pubnub::String channel_id) const
{
    Json channel_data_json;

    channel_data_json.insert_or_update("id", channel_id);
    if(!channel_name.empty()) {
        channel_data_json.insert_or_update("name", channel_name);
    }
    if(!description.empty()) {
        channel_data_json.insert_or_update("description", description);
    }
    if(!custom_data_json.empty()) {
        Json custom_json = Json::parse(custom_data_json);
        channel_data_json.insert_or_update("custom", custom_json);
    }
    if(!updated.empty()) {
        channel_data_json.insert_or_update("updated", updated);
    }
    if(!status.empty()) {
        channel_data_json.insert_or_update("status", status);
    }
    if(!type.empty()) {
        channel_data_json.insert_or_update("type", type);
    }

    return channel_data_json.dump();
}

ChannelEntity ChannelEntity::from_json(Json channel_json) {
    ChannelEntity new_entity;
    if (channel_json.contains("channel") && !channel_json["channel"].is_null())
    {
        new_entity.channel_name = channel_json["channel"].get_string("name").value_or(Pubnub::String(""));
        new_entity.description = channel_json["channel"].get_string("description").value_or(Pubnub::String(""));
    }
    new_entity.custom_data_json = channel_json.contains("custom") ? channel_json["custom"].dump() : Pubnub::String("");
    new_entity.updated = channel_json.get_string("updated").value_or(Pubnub::String(""));
    new_entity.status = channel_json.get_string("status").value_or(Pubnub::String(""));
    new_entity.type =  channel_json.get_string("type").value_or(Pubnub::String(""));

    return new_entity;
}

ChannelEntity ChannelEntity::from_channel_response(Json response) {
    return ChannelEntity{
        response.get_string("name").value_or(Pubnub::String("")),
        response.get_string("description").value_or(Pubnub::String("")),
        response.contains("custom") ? response["custom"].dump() : Pubnub::String(""),
        response.get_string("updated").value_or(Pubnub::String("")),
        response.get_string("status").value_or(Pubnub::String("")),
        response.get_string("type").value_or(Pubnub::String(""))
    };
}

ChannelEntity ChannelEntity::from_base_and_updated_channel(ChannelEntity base_channel, ChannelEntity updated_channel)
{
    ChannelEntity new_entity;
    new_entity.channel_name = updated_channel.channel_name.empty() ? base_channel.channel_name : updated_channel.channel_name;
    new_entity.description = updated_channel.description.empty() ? base_channel.description : updated_channel.description;
    new_entity.custom_data_json = updated_channel.custom_data_json.empty() ? base_channel.custom_data_json : updated_channel.custom_data_json;
    new_entity.updated = updated_channel.updated.empty() ? base_channel.updated : updated_channel.updated;
    new_entity.status = updated_channel.status.empty() ? base_channel.status : updated_channel.status;
    new_entity.type = updated_channel.type.empty() ? base_channel.type : updated_channel.type;
    return new_entity;
}

ChannelEntity ChannelEntity::pin_message(std::pair<ChannelId, MessageTimetoken> channel_message) const {
    auto custom_data = this->custom_data_json.empty() ? "{}" :  this->custom_data_json;

    Json custom_data_json = Json::parse(custom_data);

    custom_data_json.insert_or_update("pinnedMessageChannelID", channel_message.first);
    custom_data_json.insert_or_update("pinnedMessageTimetoken", channel_message.second);

    return ChannelEntity{
        this->channel_name,
        this->description,
        custom_data_json.dump(),
        this->updated,
        this->status,
        this->type
    };
}

ChannelEntity ChannelEntity::unpin_message() const {
    if (this->custom_data_json.empty()) {
        return *this;
    }

    Json custom_data_json = Json::parse(this->custom_data_json);

    custom_data_json.erase("pinnedMessageTimetoken");
    custom_data_json.erase("pinnedMessageChannelID");

    return ChannelEntity{
        this->channel_name,
        this->description,
        custom_data_json.dump(),
        this->updated,
        this->status,
        this->type
    };
}
