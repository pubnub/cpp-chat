#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <vector>
#include <future>
#include <functional>

#include "string.hpp"
#include "export.hpp"
#include "enums.hpp"
#include "chat/user.hpp"
#include "chat/channel.hpp"
#include "chat/message.hpp"
#include "infra/pubnub.hpp"


extern "C" {
    #include "core/pubnub_api_types.h"
}

class PubNub;

namespace Pubnub
{
    struct PubnubRestrictionsData
    {
        bool ban;
        bool mute;
        Pubnub::String reason;
    };

    class Chat
    {
    public:
        PN_CHAT_EXPORT Chat(Pubnub::String publish_key, Pubnub::String subscribe_key, Pubnub::String user_id);

       //TO DELETE, just for testing
        PN_CHAT_EXPORT void publish_message(Pubnub::String channel, Pubnub::String message);

        // TODO: Remove this function and use the constructor instead - keep it for now for compatibility
        PN_CHAT_EXPORT void init(Pubnub::String in_publish_key, Pubnub::String in_subscribe_key, Pubnub::String in_user_id);
        
        PN_CHAT_EXPORT void deinit();

        /* CHANNELS*/

        PN_CHAT_EXPORT Pubnub::Channel create_public_conversation(Pubnub::String channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Pubnub::Channel update_channel(Pubnub::String channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Channel get_channel(Pubnub::String channel_id);
        PN_CHAT_EXPORT std::vector<Channel> get_channels(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        PN_CHAT_EXPORT void delete_channel(Pubnub::String channel_id);

        /* USERS */

        PN_CHAT_EXPORT Pubnub::User create_user(Pubnub::String user_id, ChatUserData user_data);
        PN_CHAT_EXPORT Pubnub::User get_user(Pubnub::String user_id);
        PN_CHAT_EXPORT std::vector<User> get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        PN_CHAT_EXPORT Pubnub::User update_user(Pubnub::String user_id, ChatUserData user_data);
        PN_CHAT_EXPORT void delete_user(Pubnub::String user_id);

        /* MODERATION */

        PN_CHAT_EXPORT void set_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, Pubnub::PubnubRestrictionsData restrictions);
        PN_CHAT_EXPORT PubnubRestrictionsData get_channel_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end);

        /* PRESENCE */

        PN_CHAT_EXPORT std::vector<Pubnub::String> where_present(Pubnub::String user_id);
        PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present(Pubnub::String channel_id);
        PN_CHAT_EXPORT bool is_present(Pubnub::String user_id, Pubnub::String channel_id);

        /* EVENTS */

        PN_CHAT_EXPORT void emit_chat_event(pubnub_chat_event_type chat_event_type, Pubnub::String channel_id, Pubnub::String payload);
        //PN_CHAT_EXPORT void listen_for_events(Pubnub::String channel_id, )


        // TODO: I'm totally sure that we should handle need for pubnub context in a different way
        PubNub& get_pubnub_context(){return this->pubnub;};

        //TODO: These functions shouldn't be used by end users. Maybe make them "friend"
        void subscribe_to_channel(Pubnub::String channel_id);
        std::vector<Pubnub::String> subscribe_to_channel_and_get_last_messages(Pubnub::String channel_id);
        void unsubscribe_from_channel(Pubnub::String channel_id);
        std::vector<Pubnub::String> unsubscribe_from_channel_and_get_last_messages(Pubnub::String channel_id);

        const Pubnub::String internal_moderation_prefix = "PUBNUB_INTERNAL_MODERATION_";
        const Pubnub::String internal_admin_channel = "PUBNUB_INTERNAL_ADMIN_CHANNEL";

    

    private:
        PubNub pubnub;

        Channel create_channel(String channel_id, ChatChannelData channel_data);

        /* HELPERS */

        Chat(PubNub pubnub);
        inline Pubnub::String const bool_to_string(bool b)
        {
            return b ? "true" : "false";
        }

    };
}
#endif /* CHAT_H */
