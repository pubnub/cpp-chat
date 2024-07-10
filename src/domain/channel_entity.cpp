#include "channel_entity.hpp"
#include "domain/json.hpp"

using json = nlohmann::json;

Pubnub::String ChannelEntity::get_channel_metadata_json_string(Pubnub::String channel_id)
{
    Json channel_data_json;

    channel_data_json["id"] = channel_id;
    if(!channel_name.empty())
    {
        channel_data_json["name"] = channel_name;
    }
    if(!description.empty())
    {
        channel_data_json["description"] = description.c_str();
    }
    if(!custom_data_json.empty())
    {
        json custom_json = json::parse(custom_data_json);
        channel_data_json["custom"] = custom_json;
    }
    if(!updated.empty())
    {
        channel_data_json["updated"] = updated.c_str();
    }
    if(!status.empty())
    {
        channel_data_json["status"] = status.c_str();
    }
    if(!type.empty())
    {
        channel_data_json["type"] = type.c_str();
    }

    return channel_data_json.dump();
}

ChannelEntity ChannelEntity::from_json(Json channel_json) {
    return ChannelEntity{
        .channel_name = channel_json.get_string("name").value_or(Pubnub::String()),
        .description = channel_json.get_string("description").value_or(Pubnub::String()),
        .custom_data_json = channel_json.get_string("custom").value_or(Pubnub::String()),
        .updated = channel_json.get_string("updated").value_or(Pubnub::String()),
        .status = channel_json.get_string("status").value_or(Pubnub::String()),
        .type = channel_json.get_string("type").value_or(Pubnub::String())
    };
}
