#include <iostream>
#include "chat.h"

extern "C" {
#include "core/pubnub_alloc.h"
#include "core/pubnub_pubsubapi.h"
#include "core/pubnub_ntf_sync.h"
}

using namespace Pubnub;

void Chat::init()
{
    std::cout << "Init Chat SDK\n";
    
    ctx_pub = pubnub_alloc();
    pubnub_init(ctx_pub, publish_key, subscribe_key);
    pubnub_set_user_id(ctx_pub, user_id);

    std::cout << "Chat SDK Initialized\n";
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

void Chat::deinit()
{
    std::cout << "Deinit Chat SDK\n";

    pubnub_free(ctx_pub);

    std::cout << "Chat SDK deinitialized\n";
}
