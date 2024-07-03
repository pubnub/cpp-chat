#include "channel_entity.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Pubnub::String ChannelEntity::get_channel_metadata_json_string()
{
    json channel_data_json;

    channel_data_json["id"] = channel_id.c_str();
    if(!channel_name.empty())
    {
        channel_data_json["name"] = channel_name.c_str();
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