#ifndef PN_CHAT_C_USER_HPP
#define PN_CHAT_C_USER_HPP

#include "chat.hpp"
#include "chat/user.hpp"
#include "export.hpp"
#include "extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_user_create_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_data_json,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_destroy(Pubnub::User* user);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::User* pn_user_update_dirty(
        Pubnub::User* user,
        const char* user_data_json,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_delete_user(Pubnub::User* user);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_set_restrictions(
        Pubnub::User* user,
        const char* channel_id,
        bool ban_user,
        bool mute_user,
        const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_report(
        Pubnub::User* user,
        const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_where_present(Pubnub::User* user, char* result_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_user_is_present_on(Pubnub::User* user, const char* channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_user_id(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_user_name(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_external_id(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_profile_url(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_email(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_custom_data(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_status(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_user_get_data_type(Pubnub::User* user, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_get_channel_restrictions(
        Pubnub::User* user,
        const char* user_id,
        const char* channel_id,
        int limit,
        const char* start,
        const char* end,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_user_get_memberships(
        Pubnub::User* user,
        int limit,
        const char* start,
        const char* end,
        char* result);

#endif // PN_CHAT_C_USER_HPP