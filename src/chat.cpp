#include <iostream>
#include "chat.h"

extern "C" {
#include "core/pubnub_alloc.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_ntf_sync.h"
}

using namespace Pubnub;

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
        std::cout << "Failed to allocate Pubnub context" << std::endl;
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

Pubnub::Channel* Chat::create_public_conversation(std::string channel_id, pubnub_chat_channel_data channel_data)
{
    if(channel_id.empty())
    {
        std::cout << "Failed to create public conversation, channel_id is empty" << std::endl;
        return;
    }

    Channel* channel_ptr = new Channel;
    channel_ptr->init(channel_id, channel_data);

    return channel_ptr;
}