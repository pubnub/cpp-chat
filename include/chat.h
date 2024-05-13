#ifndef CHAT_H
#define CHAT_H

#include <string>
#include "export.hpp"
#include "chat/user.h"
#include "chat/channel.h"
#include "chat/message.h"


extern "C" {
    #include "core/pubnub_api_types.h"
}

namespace Pubnub
{
    PN_CHAT_EXPORT class Chat
    {
        public:

        Chat(){};
        ~Chat(){};

        PN_CHAT_EXPORT void init(std::string in_publish_key, std::string in_subscribe_key, std::string in_user_id);
        PN_CHAT_EXPORT void init(const char* in_publish_key, const char* in_subscribe_key, const char* in_user_id);
        
        PN_CHAT_EXPORT void deinit();

        //TO DELETE, just for testing
        PN_CHAT_EXPORT void publish_message(std::string channel, std::string message);
        PN_CHAT_EXPORT void publish_message(const char* channel, const char* message);

        Pubnub::Channel* create_public_conversation(std::string channel_id, pubnub_chat_channel_data channel_data);


        private:
        pubnub_t *ctx_pub;
        const char* publish_key;
        const char* subscribe_key;
        const char* user_id;
    };
}
#endif /* CHAT_H */
