#ifndef PN_CHAT_CHAT_HPP
#define PN_CHAT_CHAT_HPP

#include "string.hpp"
#include "presentation/channel.hpp"
#include <memory>

class ChatService;
class ChannelService;
class UserService;

namespace Pubnub {

    class Chat {
        public:
            PN_CHAT_EXPORT Chat(String publish_key, String subscribe_key, String user_id);

            PN_CHAT_EXPORT Pubnub::Channel create_public_conversation(Pubnub::String channel_id, ChatChannelData channel_data);
        private:
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<ChannelService> channel_service;
            std::shared_ptr<UserService> user_service;
    };
}

#endif // PN_CHAT_CHAT_HPP
