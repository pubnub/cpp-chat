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

}