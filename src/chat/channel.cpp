#include <iostream>
#include "chat/channel.h"
#include "nlohmann/json.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

void Channel::init(pubnub_t* in_ctx, std::string in_channel_id, ChatChannelData in_additional_channel_data)
{
    ctx_pub = in_ctx;
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    //pubnub_set_channelmetadata(ctx_pub, channel_id.c_str(), NULL, channel_data_to_json(channel_id, channel_data).c_str());

    //now channel is fully initialized
    is_initialized = true;
}

void Channel::init(pubnub_t* in_ctx, const char* in_channel_id, ChatChannelDataChar in_additional_channel_data)
{
    std::string channel_id_string = in_channel_id;
    init(in_ctx, channel_id_string, ChatChannelData(in_additional_channel_data));
}

PN_CHAT_EXPORT void Channel::init_from_json(pubnub_t* in_ctx, std::string in_channel_id, std::string channel_data_json)
{
    init(in_ctx, in_channel_id, channel_data_from_json(channel_data_json));
}

PN_CHAT_EXPORT void Channel::init_from_json(pubnub_t* in_ctx, const char *in_channel_id, const char *channel_data_json)
{
    std::string channel_id_string = in_channel_id;
    std::string channel_data_json_string = channel_data_json;
    init_from_json(in_ctx, channel_id_string, channel_data_json_string);
}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    channel_data = in_additional_channel_data;
    pubnub_set_channelmetadata(ctx_pub, channel_id.c_str(), NULL, channel_data_to_json(channel_id, channel_data).c_str());
}

void Channel::update(ChatChannelDataChar in_additional_channel_data)
{
    update(ChatChannelData(in_additional_channel_data));
}

ChatChannelData Channel::channel_data_from_json(std::string json_string)
{
    json response_json = json::parse(json_string);

    if(response_json.is_null())
    {
        return ChatChannelData();
    }

    json channel_data_json = response_json["data"];
    ChatChannelData channel_data;

    if(channel_data_json.contains("name") )
    {
        channel_data.channel_name = channel_data_json["name"];
    }
    if(channel_data_json.contains("description") )
    {
        channel_data.description = channel_data_json["description"];
    }
    if(channel_data_json.contains("custom") )
    {
        channel_data.custom_data_json = channel_data_json["custom"];
    }
    if(channel_data_json.contains("updated") )
    {
        channel_data.updated = channel_data_json["updated"];
    }
    if(channel_data_json.contains("status") )
    {
        channel_data.status = channel_data_json["status"];
    }
    if(channel_data_json.contains("type") )
    {
        channel_data.type = channel_data_json["type"];
    }

    return channel_data;
}

std::string Channel::channel_data_to_json(std::string in_channel_id, ChatChannelData in_channel_data)
{
    json channel_data_json;

    channel_data_json["id"] = in_channel_id;
    if(in_channel_data.channel_name.empty())
    {
        channel_data_json["name"] = in_channel_data.channel_name;
    }
    if(in_channel_data.description.empty())
    {
        channel_data_json["description"] = in_channel_data.description;
    }
    if(in_channel_data.custom_data_json.empty())
    {
        channel_data_json["custom"] = in_channel_data.custom_data_json;
    }
    if(in_channel_data.updated.empty())
    {
        channel_data_json["updated"] = in_channel_data.updated;
    }
    if(in_channel_data.status != 0 )
    {
        channel_data_json["status"] = in_channel_data.status;
    }
    if(in_channel_data.type.empty())
    {
        channel_data_json["type"] = in_channel_data.type;
    }

    return channel_data_json.dump();
}