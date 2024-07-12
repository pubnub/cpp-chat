#include "channel_entity.hpp"
#include "domain/json.hpp"

using json = nlohmann::json;

Pubnub::String ChannelEntity::get_channel_metadata_json_string(Pubnub::String channel_id)
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
        channel_json.get_string("name").value_or(Pubnub::String()),
        channel_json.get_string("description").value_or(Pubnub::String()),
        channel_json.get_string("custom").value_or(Pubnub::String()),
        channel_json.get_string("updated").value_or(Pubnub::String()),
        channel_json.get_string("status").value_or(Pubnub::String()),
        channel_json.get_string("type").value_or(Pubnub::String())
    };
}
