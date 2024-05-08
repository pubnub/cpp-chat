#pragma once

//#include  C-Core
#include <string>

class ChatSDK
{
    public:

    void Init();
    void PublishMessage(std::string Channel, std::string Message);

    private:
    //pubnub_t *ctx_pub;
    const char* PublishKey = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    const char* SubscribeKey = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
    std::string UserID = "TestUser";
};