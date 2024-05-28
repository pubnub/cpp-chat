#include "chat.hpp"
#include "export.hpp"
#include "extern.hpp"
#include "chat/channel.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id); 

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_delete(Pubnub::Chat* chat);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_send_message(
        Pubnub::Chat* chat,
        const char* message); 

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_create_public_conversation(
        Pubnub::Chat* chat,
        const char* channel_id,
        Pubnub::ChatChannelDataChar channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_update_channel(
        Pubnub::Chat* chat,
        std::string channel_id,
        Pubnub::ChatChannelDataChar channel_data);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

// TODO: get channels...

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_delete_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_set_restrictions(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id,
        bool ban_user, 
        bool mute_user,
        const char* reason);

