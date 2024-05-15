#include <iostream>
#include "chat.h"
#include "nlohmann/json.hpp"

extern "C" {
#include "core/pubnub_alloc.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_ntf_sync.h"
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

void Chat::init(std::string in_publish_key, std::string in_subscribe_key, std::string in_user_id)
{
    init(in_publish_key.c_str(), in_subscribe_key.c_str(), in_user_id.c_str());
}

void Chat::init(const char* in_publish_key, const char* in_subscribe_key, const char* in_user_id)
{
    publish_key = in_publish_key;
    subscribe_key = in_subscribe_key;
    user_id = in_user_id;
    ctx_pub = pubnub_alloc();

    if (NULL == ctx_pub) 
    {
        std::cout << "Failed to allocate Pubnub context\n" << std::endl;
        return;
    }

    pubnub_init(ctx_pub, publish_key, subscribe_key);
    pubnub_set_user_id(ctx_pub, user_id);

}

void Chat::deinit()
{
    std::cout << "Deinit Chat SDK\n";

    pubnub_free(ctx_pub);

    std::cout << "Chat SDK deinitialized\n";
}


void Chat::publish_message(std::string channel, std::string message)
{
    publish_message(channel.c_str(), message.c_str());
}

void Chat::publish_message(const char* channel, const char*message)
{
    std::cout << "Publish message\n";

    pubnub_publish(ctx_pub, channel, message);

    pubnub_await(ctx_pub);

    std::cout << "Message published\n";
}

Pubnub::Channel* Chat::create_public_conversation(std::string channel_id, ChatChannelData channel_data)
{
    return create_public_conversation(channel_id.c_str(), ChatChannelDataChar(channel_data));
}

Pubnub::Channel* Chat::create_public_conversation(const char* channel_id, ChatChannelDataChar channel_data)
{
    if(*channel_id == 0)
    {
        std::cout << "Failed to create public conversation, channel_id is empty\n" << std::endl;
        return nullptr;
    }

    Channel* channel_ptr = new Channel;
    //channel_ptr->init(channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(std::string channel_id, ChatChannelData channel_data)
{
    return update_channel(channel_id.c_str(), ChatChannelDataChar(channel_data));
}

Pubnub::Channel* Chat::update_channel(const char* channel_id, ChatChannelDataChar channel_data)
{
    pubnub_set_channelmetadata(ctx_pub, channel_id, NULL, channel_data_to_json_char(channel_id, channel_data));

    Channel* channel_ptr = new Channel;
    channel_ptr->init(channel_id, channel_data);

    return channel_ptr;
}

PN_CHAT_EXPORT Pubnub::Channel *Pubnub::Chat::get_channel(std::string channel_id)
{
    return get_channel(channel_id.c_str());
}

PN_CHAT_EXPORT Pubnub::Channel *Pubnub::Chat::get_channel(const char *channel_id)
{
    if(channel_id == 0)
    {
        std::cout << "Failed to get channel, channel_id is empty\n" << std::endl;
        return nullptr;
    }

    auto future_response = get_channel_metadata_async(channel_id);
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        std::cout << "Failed to get channel response error\n" << std::endl;
        return nullptr;
    }

    const char* channel_response = pubnub_get(ctx_pub);

    Channel* channel_ptr = new Channel;
    channel_ptr->init_from_json(channel_id, channel_response);

    return channel_ptr;
}

const char* Chat::channel_data_to_json_char(const char* channel_id, ChatChannelDataChar channel_data)
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

std::future<pubnub_res> Pubnub::Chat::get_channel_metadata_async(const char *channel_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_channelmetadata(ctx_pub, NULL, channel_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}
