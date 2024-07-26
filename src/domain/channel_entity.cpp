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
    return ChannelEntity{
        channel_json["channel"].get_string("name").value_or(Pubnub::String("")),
        channel_json["channel"].get_string("description").value_or(Pubnub::String("")),
        channel_json.contains("custom") ? channel_json["custom"].dump() : Pubnub::String(""),
        channel_json.get_string("updated").value_or(Pubnub::String("")),
        channel_json.get_string("status").value_or(Pubnub::String("")),
        channel_json.get_string("type").value_or(Pubnub::String(""))
    };
}

ChannelEntity ChannelEntity::from_channel_response(Json response) {
    if(response.is_null())
    {
        throw std::runtime_error("can't create channel from response, response is incorrect");
    }

    //In most responses this data field is an array but in some cases (for example in get_channel) it's just an object.
    Json channel_data_json = response["data"].is_array() ? response["data"][0] : response["data"];

    if(channel_data_json.is_null())
    {
        throw std::runtime_error("can't create channel from response, response doesn't have data field");
    }

    return ChannelEntity::from_json(channel_data_json);
}

ChannelEntity ChannelEntity::pin_message(std::pair<ChannelId, MessageTimetoken> channel_message) const {
    auto custom_data = this->custom_data_json.empty() ? "{}" :  this->custom_data_json;

    json custom_data_json = json::parse(custom_data);

    custom_data_json["pinnedMessageChannelID"] = channel_message.first.c_str();
    custom_data_json["pinnedMessageTimetoken"] = channel_message.second.c_str();

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
