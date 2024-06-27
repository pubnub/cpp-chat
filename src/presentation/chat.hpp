#ifndef PN_CHAT_CHAT_HPP
#define PN_CHAT_CHAT_HPP

#include "string.hpp"
#include "presentation/channel.hpp"
#include <memory>

class ChatService;

namespace Pubnub {
    struct ChatChannelData
    {
        Pubnub::String channel_name;
        Pubnub::String description;
        Pubnub::String custom_data_json;
        Pubnub::String updated;
        Pubnub::String status;
        Pubnub::String type;
    };

    class Chat {
        public:
            Chat(String publish_key, String subscribe_key, String user_id);

            PN_CHAT_EXPORT Pubnub::Channel create_public_conversation(Pubnub::String channel_id, ChatChannelData channel_data);
        private:
            std::shared_ptr<ChatService> chat_service;
    };
}

#endif // PN_CHAT_CHAT_HPP
