#ifndef PN_CHAT_C_CHANNEL_H
#define PN_CHAT_C_CHANNEL_H

#include "callback_handle.hpp"
#include "chat.hpp"
#include "message_draft.hpp"
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

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_connect(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_disconnect(Pubnub::Channel* channel, char* result_messages);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_join(Pubnub::Channel* channel, const char* additional_params, char* result_messages);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_leave(Pubnub::Channel* channel, char* result_messages);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_delete_channel(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_set_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        bool ban,
        bool mute,
        const char* reason);

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
        Pubnub::User* user,
        char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_members(
        Pubnub::Channel* channel,
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
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

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_emit_user_mention(Pubnub::Channel* channel, const char* user_id, const char* timetoken, const char* text);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_channel_update_with_base(Pubnub::Channel* channel, Pubnub::Channel* base_channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_user_suggestions(Pubnub::Channel* channel, const char* text, int limit, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_send_text_dirty(
    Pubnub::Channel* channel,
    const char* message,
    bool store_in_history,
    bool send_by_post,
    const char* meta,
    int mentioned_users_length,
    int* mentioned_users_indexes,
    Pubnub::User** mentioned_users,
    Pubnub::Message* quoted_message);


PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_channel_get_users_restrictions(Pubnub::Channel* channel, const char* sort, int limit, const char* next, const char* prev, char* result);

#define PN_MESSAGE_DRAFT_SUGGESTION_SOURCE int
#define PN_MESSAGE_DRAFT_USER_SUGGESTION_SOURCE_CHANNEL 0
#define PN_MESSAGE_DRAFT_USER_SUGGESTION_SOURCE_GLOBAL 1

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::MessageDraft* pn_channel_create_message_draft_dirty(Pubnub::Channel* channel,
    int user_suggestion_source,
    bool is_typing_indicator_triggered,
    int user_limit,
    int channel_limit);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_stream_updates(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_get_typing(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_stream_presence(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_stream_read_receipts(Pubnub::Channel* channel);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::CallbackHandle* pn_channel_stream_message_reports(Pubnub::Channel* channel);

#endif // PN_CHAT_C_CHANNEL_H

