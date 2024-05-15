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
    init(in_ctx, in_channel_id.c_str(), ChatChannelDataChar(in_additional_channel_data));
}

void Channel::init(pubnub_t* in_ctx, const char* in_channel_id, ChatChannelDataChar in_additional_channel_data)
{
    ctx_pub = in_ctx;
    channel_id = in_channel_id;
    channel_data = in_additional_channel_data;

    pubnub_set_channelmetadata(ctx_pub, channel_id, NULL, channel_data_to_json_char(channel_id, channel_data));

    //now channel is fully initialized
    is_initialized = true;
}

PN_CHAT_EXPORT void Channel::init_from_json(pubnub_t* in_ctx, std::string in_channel_id, std::string channel_data_json)
{
    init_from_json(in_ctx, in_channel_id.c_str(), channel_data_json.c_str());
}

PN_CHAT_EXPORT void Channel::init_from_json(pubnub_t* in_ctx, const char *in_channel_id, const char *channel_data_json)
{
    init(in_ctx, in_channel_id, channel_data_from_json_char(channel_data_json));
}

void Channel::update(ChatChannelData in_additional_channel_data)
{
    update(ChatChannelDataChar(in_additional_channel_data));
}

void Channel::update(ChatChannelDataChar in_additional_channel_data)
{
    channel_data = in_additional_channel_data;

    pubnub_set_channelmetadata(ctx_pub, channel_id, NULL, channel_data_to_json_char(channel_id, channel_data));
}

ChatChannelDataChar Channel::channel_data_from_json_char(const char *json_char)
{
    json channel_data_json = json::parse(json_char);

    if(channel_data_json.is_null())
    {
        return ChatChannelDataChar();
    }

    ChatChannelDataChar channel_data;

    if(channel_data_json.contains("name") )
    {
        channel_data.channel_name = ((std::string)channel_data_json["name"]).c_str();
    }
    if(channel_data_json.contains("description") )
    {
        channel_data.description = ((std::string)channel_data_json["description"]).c_str();
    }
    if(channel_data_json.contains("custom") )
    {
        channel_data.custom_data_json = ((std::string)channel_data_json["custom"]).c_str();
    }
    if(channel_data_json.contains("updated") )
    {
        channel_data.updated = ((std::string)channel_data_json["updated"]).c_str();
    }
    if(channel_data_json.contains("status") )
    {
        channel_data.status = ((std::string)channel_data_json["status"]).c_str();
    }
    if(channel_data_json.contains("type") )
    {
        channel_data.type = ((std::string)channel_data_json["type"]).c_str();
    }

    return channel_data;
}

const char* Channel::channel_data_to_json_char(const char* channel_id, ChatChannelDataChar channel_data)
{
    json channel_data_json;

    channel_data_json["id"] = channel_id;
    if(*channel_data.channel_name == 0 )
    {
        channel_data_json["name"] = channel_data.channel_name;
    }
    if(*channel_data.description == 0 )
    {
        channel_data_json["description"] = channel_data.description;
    }
    if(*channel_data.custom_data_json == 0 )
    {
        channel_data_json["custom"] = channel_data.custom_data_json;
    }
    if(*channel_data.updated == 0 )
    {
        channel_data_json["updated"] = channel_data.updated;
    }
    if(*channel_data.status == 0 )
    {
        channel_data_json["status"] = channel_data.status;
    }
    if(*channel_data.type == 0 )
    {
        channel_data_json["type"] = channel_data.type;
    }

    return channel_data_json.dump().c_str();
}
