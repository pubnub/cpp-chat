#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <future>

#include "export.hpp"
#include "chat/user.hpp"
#include "chat/channel.hpp"
#include "chat/message.hpp"


extern "C" {
    #include "core/pubnub_api_types.h"
}

namespace Pubnub
{
    enum pubnub_chat_event_type
    {
        PCET_TYPING,
        PCET_REPORT,
        PCET_RECEPIT,
        PCET_MENTION,
        PCET_INVITE,
        PCET_CUSTOM,
        PCET_MODERATION
    };

    enum pubnub_chat_message_type
    {
        PCMT_TEXT
    };

    class Chat
    {
        public:

        //TO DELETE, just for testing
        PN_CHAT_EXPORT void publish_message(std::string channel, std::string message);
        PN_CHAT_EXPORT void publish_message(const char* channel, const char* message);

        Chat(){};
        ~Chat()
        {
            deinit();
        };

        PN_CHAT_EXPORT void init(std::string in_publish_key, std::string in_subscribe_key, std::string in_user_id);
        PN_CHAT_EXPORT void init(const char* in_publish_key, const char* in_subscribe_key, const char* in_user_id);
        
        PN_CHAT_EXPORT void deinit();

        /* CHANNELS*/

        PN_CHAT_EXPORT Pubnub::Channel* create_public_conversation(std::string channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Pubnub::Channel* create_public_conversation(const char* channel_id, ChatChannelDataChar channel_data);

        PN_CHAT_EXPORT Pubnub::Channel* update_channel(std::string channel_id, ChatChannelData channel_data);
        PN_CHAT_EXPORT Pubnub::Channel* update_channel(const char* channel_id, ChatChannelDataChar channel_data);

        PN_CHAT_EXPORT Channel get_channel(std::string channel_id);
        PN_CHAT_EXPORT Channel get_channel(const char* channel_id);

        PN_CHAT_EXPORT void delete_channel(std::string channel_id);
        PN_CHAT_EXPORT void delete_channel(const char* channel_id);

        PN_CHAT_EXPORT void set_restrictions(std::string in_user_id, std::string in_channel_id, bool ban_user, bool mute_user, std::string reason = "");
        PN_CHAT_EXPORT void set_restrictions(const char* in_user_id, const char* in_channel_id, bool ban_user, bool mute_user, const char* reason = NULL);



        pubnub_t* get_pubnub_context(){return ctx_pub;};

        //TODO: These functions shouldn't be used by end users. Maybe make them "friend"
        void subscribe_to_channel(const char* channel_id);
        void unsubscribe_from_channel(const char* channel_id);

        const std::string internal_moderation_prefix = "PUBNUB_INTERNAL_MODERATION_";

        private:

        pubnub_t *ctx_pub;
        pubnub_t *ctx_sub;
        const char* publish_key;
        const char* subscribe_key;
        const char* user_id;

        std::future<pubnub_res> get_channel_metadata_async(const char* channel_id);

        void emit_chat_event(pubnub_chat_event_type chat_event_type, std::string channel_id, std::string payload);

        /* HELPERS */

        std::string get_string_from_event_type(pubnub_chat_event_type chat_event_type);
        inline std::string const bool_to_string(bool b)
        {
            return b ? "true" : "false";
        }

    };
}
#endif /* CHAT_H */
