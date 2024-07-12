#ifndef PN_CHAT_C_CHAT_HPP
#define PN_CHAT_C_CHAT_HPP

#include "chat.hpp"
#include "export.hpp"
#include "extern.hpp"
#include "chat/channel.hpp"
#include "chat/user.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id); 

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_delete(Pubnub::Chat* chat);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_create_public_conversation_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_update_channel_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

// TODO: get channels...

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_delete_channel(
        Pubnub::Chat* chat,
        const char* channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_set_restrictions(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id,
        bool ban_user, 
        bool mute_user,
        const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_chat_create_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_chat_get_user(
        Pubnub::Chat* chat,
        const char* user_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_users(
        Pubnub::Chat* chat,
        const char* include,
        const int limit,
        const char* start,
        const char* end,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_chat_update_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_delete_user(
        Pubnub::Chat* chat,
        const char* user_id);

// Hacky way to get the messages
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_updates(
        Pubnub::Chat* chat,
        char* messages_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_clear_string(char* str);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_chat_is_present(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_who_is_present(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_channels(
        Pubnub::Chat* chat,
        const char* include,
        const int limit,
        const char* start,
        const char* end,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_listen_for_events(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pin_message_to_channel(Pubnub::Chat* chat, Pubnub::Message* message, Pubnub::Channel* channel);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult unpin_message_from_channel(Pubnub::Chat* chat, Pubnub::Channel* channel);

#endif // PN_CHAT_C_CHAT_HPP