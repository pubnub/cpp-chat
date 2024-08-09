#ifndef PN_CHAT_C_MESSAGE_H
#define PN_CHAT_C_MESSAGE_H

#include "message.hpp"
#include "chat.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_delete(Pubnub::Message* message);

// TODO: ctors with json check

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_message_edit_text(
        Pubnub::Message* message,
        const char* text);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_text(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_delete_message(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_message_deleted(Pubnub::Message* message);

// TODO: streams

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_timetoken(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT int pn_message_get_data_type(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_data_text(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_data_channel_id(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_data_user_id(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_data_meta(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_get_data_message_actions(Pubnub::Message* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_pin(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_get_reactions(Pubnub::Message* message, char* reactions_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_message_toggle_reaction(Pubnub::Message* message, const char* reaction);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_message_has_user_reaction(Pubnub::Message* message, const char* reaction);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_report(Pubnub::Message* message, const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadChannel* pn_message_create_thread(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_message_has_thread(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadChannel* pn_message_get_thread(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_remove_thread(Pubnub::Message* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_message_update_with_base_message(Pubnub::Message* message, Pubnub::Message* base_message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_mentioned_users(Pubnub::Message* message, Pubnub::Chat* chat, char* result);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_referenced_channels(Pubnub::Message* message, Pubnub::Chat* chat, char* result);
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_message_quoted_message(Pubnub::Message* message);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_message_text_links(Pubnub::Message* message, char* result);

#endif // PN_CHAT_C_MESSAGE_H
