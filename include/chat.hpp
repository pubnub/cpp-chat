#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <future>

#include "string.hpp"
#include "export.hpp"
#include "chat/user.hpp"
#include "chat/channel.hpp"
#include "chat/message.hpp"


extern "C" {
    #include "core/pubnub_api_types.h"
}

namespace Pubnub
{
    enum pubnub_chat_event_type : uint8_t
    {
        PCET_TYPING,
        PCET_REPORT,
        PCET_RECEPIT,
        PCET_MENTION,
        PCET_INVITE,
        PCET_CUSTOM,
        PCET_MODERATION
    };

    enum pubnub_chat_message_type : uint8_t
    {
        PCMT_TEXT
    };

    class Chat
    {
        public:

        //TO DELETE, just for testing
        PN_CHAT_EXPORT void publish_message(Pubnub::String channel, Pubnub::String message);

        Chat(){};
        ~Chat()
        {
            deinit();
        };

        PN_CHAT_EXPORT void init(Pubnub::String in_publish_key, Pubnub::String in_subscribe_key, Pubnub::String in_user_id);
        
        PN_CHAT_EXPORT void deinit();

        /* CHANNELS*/

        PN_CHAT_EXPORT Pubnub::Channel* create_public_conversation(Pubnub::String channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Pubnub::Channel* update_channel(Pubnub::String channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Channel get_channel(Pubnub::String channel_id);
        PN_CHAT_EXPORT std::vector<Channel> get_channels(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        PN_CHAT_EXPORT void delete_channel(Pubnub::String channel_id);
        PN_CHAT_EXPORT void set_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, bool ban_user, bool mute_user, Pubnub::String reason = "");

        /* USERS */

        PN_CHAT_EXPORT Pubnub::User create_user(Pubnub::String user_id, ChatUserData user_data);
        PN_CHAT_EXPORT Pubnub::User get_user(Pubnub::String user_id);


        //Just to test new string
        PN_CHAT_EXPORT Pubnub::Message get_message(String MessageTest);

        pubnub_t* get_pubnub_context(){return ctx_pub;};

        //TODO: These functions shouldn't be used by end users. Maybe make them "friend"
        void subscribe_to_channel(Pubnub::String channel_id);
        void unsubscribe_from_channel(Pubnub::String channel_id);

        const Pubnub::String internal_moderation_prefix = "PUBNUB_INTERNAL_MODERATION_";

        private:

        pubnub_t *ctx_pub;
        pubnub_t *ctx_sub;
        const char* publish_key;
        const char* subscribe_key;
        const char* user_id;

        std::future<pubnub_res> get_channel_metadata_async(const char* channel_id);
        std::future<pubnub_res> get_all_channels_metadata_async(const char* include, int limit, const char* start, const char* end);
        std::future<pubnub_res> get_uuid_metadata_async(const char* user_id);

        void emit_chat_event(pubnub_chat_event_type chat_event_type, Pubnub::String channel_id, Pubnub::String payload);

        /* HELPERS */

        Pubnub::String get_string_from_event_type(pubnub_chat_event_type chat_event_type);
        inline Pubnub::String const bool_to_string(bool b)
        {
            return b ? "true" : "false";
        }

    };
}
#endif /* CHAT_H */
