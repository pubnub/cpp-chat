#ifndef PN_CHAT_C_CHAT_HPP
#define PN_CHAT_C_CHAT_HPP

#include "chat.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "channel.hpp"
#include "user.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id,
        const char* auth_key,
        int typing_timeout,
        int typing_timeout_difference,
        int store_user_activity_interval,
        bool store_user_activity_timestamps,
        float rate_limit_factor,
        int direct_conversation_rate_limit,
        int group_conversation_rate_limit,
        int public_conversation_rate_limit,
        int unknown_conversation_rate_limit);

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

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CreatedChannelWrapper* pn_chat_create_direct_conversation_dirty(
    Pubnub::Chat* chat,
    Pubnub::User* user,
    const char* channel_id,
    char* channel_name,
    char* channel_description,
    char* channel_custom_data_json,
    char* channel_updated,
    char* channel_status,
    char* channel_type
);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CreatedChannelWrapper*
    pn_chat_create_direct_conversation_dirty_with_membership_data(
        Pubnub::Chat* chat,
        Pubnub::User* user,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type,
        char* membership_custom_json,
        char* membership_type,
        char* membership_status
    );

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CreatedChannelWrapper* pn_chat_create_group_conversation_dirty(
    Pubnub::Chat* chat,
    Pubnub::User** users,
    int users_length,
    const char* channel_id,
    char* channel_name,
    char* channel_description,
    char* channel_custom_data_json,
    char* channel_updated,
    char* channel_status,
    char* channel_type
);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CreatedChannelWrapper*
    pn_chat_create_group_conversation_dirty_with_membership_data(
        Pubnub::Chat* chat,
        Pubnub::User** users,
        int users_length,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type,
        char* membership_custom_json,
        char* membership_type,
        char* membership_status
    );

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_chat_get_created_channel_wrapper_channel(
    Pubnub::CreatedChannelWrapper* wrapper);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_chat_get_created_channel_wrapper_host_membership(
    Pubnub::CreatedChannelWrapper* wrapper);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_created_channel_wrapper_invited_memberships(
    Pubnub::CreatedChannelWrapper* wrapper, char* result_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_chat_dispose_created_channel_wrapper(
    Pubnub::CreatedChannelWrapper* wrapper);

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
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
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
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_chat_listen_for_events(
        Pubnub::Chat* chat,
        const char* channel_id,
        Pubnub::pubnub_chat_event_type event_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pin_message_to_channel(Pubnub::Chat* chat, Pubnub::Message* message, Pubnub::Channel* channel);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult unpin_message_from_channel(Pubnub::Chat* chat, Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_forward_message(Pubnub::Chat* chat, Pubnub::Message* message, Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_emit_event(Pubnub::Chat* chat, Pubnub::pubnub_chat_event_type chat_event_type, const char* channel_id, const char* payload);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_unread_messages_counts(Pubnub::Chat* chat, const char* filter, const char* sort, int limit, const char* next, const char* prev, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_channel_suggestions(Pubnub::Chat* chat, const char* text, int limit, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_mark_all_messages_as_read(Pubnub::Chat* chat, const char* filter, const char* sort, int limit, const char* next, const char* prev, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_events_history(Pubnub::Chat* chat, const char* channel_id, const char* start_timetoken, const char* end_timetoken, int count, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_current_user_mentions(Pubnub::Chat* chat, const char* start_timetoken, const char* end_timetoken, int count, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_chat_get_user_suggestions(Pubnub::Chat* chat, char* text, int limit, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_chat_current_user(Pubnub::Chat* chat);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_pam_can_i(Pubnub::Chat* chat, Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const char* resource_name);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_pam_parse_token(Pubnub::Chat* chat, const char* token, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_pam_set_auth_token(Pubnub::Chat* chat, const char* token);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_pam_set_pubnub_origin(Pubnub::Chat* chat, const char* origin);

#endif // PN_CHAT_C_CHAT_HPP
