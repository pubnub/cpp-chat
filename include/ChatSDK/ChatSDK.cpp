#include <iostream>

#include "ChatSDK.h"

void ChatSDK::Init()
{
    std::cout << "Init Chat SDK\n";
    
    //ctx_pub = pubnub_alloc();
    //pubnub_init(ctx_pub, PublishKey, SubscribeKey);
    //pubnub_set_user_id(ctx_pub, UserID.c_str())

    std::cout << "Chat SDK Initialized\n";
}

void ChatSDK::PublishMessage(std::string Channel, std::string Message)
{
    std::cout << "Publish message\n";

    //pubnub_publish(ctx_pub, Channel.c_str(), Message.c_str())

    std::cout << "Message published\n";
}

void ChatSDK::Deinit()
{
    std::cout << "Deinit Chat SDK\n";

    //pubnub_free(ctx_pub);

    std::cout << "Chat SDK deinitialized\n";
}