#ifndef PN_CHAT_CHAT_HPP
#define PN_CHAT_CHAT_HPP

#include "string.hpp"
#include "presentation/channel.hpp"
#include "presentation/message.hpp"
#include "presentation/user.hpp"
#include <memory>
#include <vector>

class ChatService;
class ChannelService;
class UserService;

namespace Pubnub {

    class Chat {
        public:
            PN_CHAT_EXPORT Chat(String publish_key, String subscribe_key, String user_id);

            /* CHANNELS */

            PN_CHAT_EXPORT Pubnub::Channel create_public_conversation(Pubnub::String channel_id, ChatChannelData channel_data);
            PN_CHAT_EXPORT Channel get_channel(Pubnub::String channel_id);
            PN_CHAT_EXPORT std::vector<Channel> get_channels(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
            PN_CHAT_EXPORT Pubnub::Channel update_channel(Pubnub::String channel_id, ChatChannelData channel_data);
            PN_CHAT_EXPORT void delete_channel(Pubnub::String channel_id);
            PN_CHAT_EXPORT void pin_message_to_channel(Pubnub::Message message, Pubnub::Channel channel);
            PN_CHAT_EXPORT void unpin_message_from_channel(Pubnub::Channel channel);

            /* USERS */

            PN_CHAT_EXPORT Pubnub::User create_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
            PN_CHAT_EXPORT Pubnub::User get_user(Pubnub::String user_id);
            PN_CHAT_EXPORT std::vector<User> get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
            PN_CHAT_EXPORT Pubnub::User update_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
            PN_CHAT_EXPORT void delete_user(Pubnub::String user_id);

            /* PRESENCE */

            PN_CHAT_EXPORT std::vector<Pubnub::String> where_present(Pubnub::String user_id);
            PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present(Pubnub::String channel_id);
            PN_CHAT_EXPORT bool is_present(Pubnub::String user_id, Pubnub::String channel_id);



        private:
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<ChannelService> channel_service;
            std::shared_ptr<UserService> user_service;
    };
}

#endif // PN_CHAT_CHAT_HPP
