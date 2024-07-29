#ifndef PN_CHAT_C_CHANNEL_H
#define PN_CHAT_C_CHANNEL_H

#include "chat.hpp"
#include "channel.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_delete(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_new_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_update_dirty(
        Pubnub::Channel* channel,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_connect(Pubnub::Channel* channel, char* messages_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_disconnect(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_join(Pubnub::Channel* channel, const char* additional_params, char* messages_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_leave(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_delete_channel(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_set_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        bool ban,
        bool mute,
        const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_send_text(
        Pubnub::Channel* channel,
        const char* message,
        Pubnub::pubnub_chat_message_type type,
        const char* metadata);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_channel_id(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_channel_name(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_description(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_custom_data_json(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_updated(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_status(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_channel_get_data_type(
        Pubnub::Channel* channel,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_channel_is_present(Pubnub::Channel* channel, const char* user_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_who_is_present(Pubnub::Channel* channel, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_user_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        const char* channel_id,
        int limit,
        const char* start,
        const char* end,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_members(
        Pubnub::Channel* channel,
        int limit,
        const char* start,
        const char* end,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_history(
        Pubnub::Channel* channel,
        const char* start,
        const char* end,
        int count,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_channel_get_message(Pubnub::Channel* channel, const char* timetoken);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Membership* pn_channel_invite_user(Pubnub::Channel* channel, Pubnub::User* user);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_invite_multiple(Pubnub::Channel* channel, Pubnub::User** users, int users_length, char* result_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_start_typing(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_stop_typing(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_pin_message(Pubnub::Channel* channel, Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_unpin_message(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_channel_get_pinned_message(Pubnub::Channel* channel);

//PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::MessageDraft* pn_channel_create_message_draft_dirty(Pubnub::Channel* channel,
//    char* user_suggestion_source,
//    bool is_typing_indicator_triggered,
//    int user_limit,
//    int channel_limit);
//
#endif // PN_CHAT_C_CHANNEL_H

