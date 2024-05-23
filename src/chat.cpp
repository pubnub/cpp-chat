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
    if(!ctx_pub)
    {return;}

    pubnub_free(ctx_pub);
    ctx_pub = nullptr;
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
    if(channel_id != NULL && *channel_id == 0)
    {
        throw std::invalid_argument("Failed to create public conversation, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(std::string channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(this, channel_id, channel_data);

    return channel_ptr;
}

Pubnub::Channel* Chat::update_channel(const char* channel_id, ChatChannelDataChar channel_data)
{
    std::string channel_id_string = channel_id;

    return update_channel(channel_id_string, ChatChannelData(channel_data));
}

Channel Pubnub::Chat::get_channel(std::string channel_id)
{
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
    channel_obj.init_from_json(this, channel_id, channel_response);

    return channel_obj;
}

Channel Pubnub::Chat::get_channel(const char *channel_id)
{
    std::string channel_id_string = channel_id;
    return get_channel(channel_id_string);
}

void Pubnub::Chat::delete_channel(std::string channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }

    pubnub_remove_channelmetadata(ctx_pub, channel_id.c_str());
}

void Pubnub::Chat::delete_channel(const char *channel_id)
{
    std::string channel_id_string = channel_id;
    delete_channel(channel_id_string);
}

std::future<pubnub_res> Pubnub::Chat::get_channel_metadata_async(const char *channel_id)
{
    return std::async(std::launch::async, [=](){
        pubnub_get_channelmetadata(ctx_pub, NULL, channel_id);
        pubnub_res response = pubnub_await(ctx_pub);
        return response; 
    });
}

void Pubnub::Chat::subscribe_to_channel(const char *channel_id)
{
}

void Pubnub::Chat::unsubscribe_from_channel(const char *channel_id)
{
}
