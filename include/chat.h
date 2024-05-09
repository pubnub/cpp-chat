#ifndef CHAT_H
#define CHAT_H

#include <string>
#include "export.hpp"

extern "C" {
    #include "core/pubnub_api_types.h"
}

class Chat
{
    public:

    DLL_EXPORT void init();
    DLL_EXPORT void publish_message(std::string channel, std::string message);
    DLL_EXPORT void deinit();

    private:
    pubnub_t *ctx_pub;
    const char* publish_key = "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf";
    const char* subscribe_key = "sub-c-2b4db8f2-c025-4a76-9e23-326123298667";
    std::string user_id = "TestUser";
};

#endif /* CHAT_H */
