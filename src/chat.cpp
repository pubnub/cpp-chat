#include <iostream>
#include "chat.hpp"
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
    printf("Init Chat SDK");
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
    printf("publish message");

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
    channel_ptr->init(ctx_pub, channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(std::string channel_id, ChatChannelData channel_data)
{
    return update_channel(channel_id.c_str(), ChatChannelDataChar(channel_data));
}

Pubnub::Channel* Chat::update_channel(const char* channel_id, ChatChannelDataChar channel_data)
{
    Channel* channel_ptr = new Channel;
    channel_ptr->init(ctx_pub, channel_id, channel_data);

    return channel_ptr;
}

PN_CHAT_EXPORT Channel Pubnub::Chat::get_channel(std::string channel_id)
{
    printf("Start get channel");
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    auto future_response = get_channel_metadata_async(channel_id.c_str());
    future_response.wait();
    pubnub_res Res = future_response.get();
    if(Res != PNR_OK)
    {
        throw std::invalid_argument("Failed to get response from server");
    }

    std::string channel_response = pubnub_get(ctx_pub);

    Channel channel_obj;
    channel_obj.init_from_json(ctx_pub, channel_id, channel_response);

    return channel_obj;
}

PN_CHAT_EXPORT Channel Pubnub::Chat::get_channel(const char *channel_id)
{
    std::string channel_id_string = channel_id;
    return get_channel(channel_id_string);
}

std::future<pubnub_res> Pubnub::Chat::get_channel_metadata_async(const char *channel_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_channelmetadata(ctx_pub, NULL, channel_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}
