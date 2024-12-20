#ifndef PN_CHAT_C_THREAD_MESSAGE_H
#define PN_CHAT_C_THREAD_MESSAGE_H

#include "chat.hpp"
#include "thread_channel.hpp"
#include "thread_message.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"
#include "c_errors.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_dispose(Pubnub::ThreadMessage* thread_message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_edit_text(
        Pubnub::ThreadMessage* message,
        const char* text);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_text(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_delete_message(Pubnub::ThreadMessage* message);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_delete_message_hard(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_thread_message_deleted(Pubnub::ThreadMessage* message);

// TODO: streams

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_get_timetoken(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT int pn_thread_message_get_data_type(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_get_data_text(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_get_data_channel_id(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_get_data_user_id(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_thread_message_get_data_meta(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_get_data_message_actions(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_pin(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_get_reactions(Pubnub::ThreadMessage* message, char* reactions_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_toggle_reaction(Pubnub::ThreadMessage* message, const char* reaction);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_thread_message_has_user_reaction(Pubnub::ThreadMessage* message, const char* reaction);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_report(Pubnub::ThreadMessage* message, const char* reason);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadChannel* pn_thread_message_create_thread(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCTribool pn_thread_message_has_thread(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadChannel* pn_thread_message_get_thread(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_remove_thread(Pubnub::ThreadMessage* message);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_update_with_base_message(Pubnub::ThreadMessage* message, Pubnub::ThreadMessage* base_message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_mentioned_users(Pubnub::ThreadMessage* message, Pubnub::Chat* chat, char* result);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_referenced_channels(Pubnub::ThreadMessage* message, Pubnub::Chat* chat, char* result);
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Message* pn_thread_message_quoted_message(Pubnub::ThreadMessage* message);
PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_text_links(Pubnub::ThreadMessage* message, char* result);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_restore(Pubnub::ThreadMessage* message);



PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::ThreadMessage* pn_thread_message_consume_and_upgrade(
        Pubnub::ThreadMessage* message,
        const char* parent_channel_id);

PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_message_unpin_from_parent_channel(Pubnub::ThreadMessage* thread_message);
PN_CHAT_EXTERN PN_CHAT_EXPORT Pubnub::Channel* pn_thread_message_pin_to_parent_channel(Pubnub::ThreadMessage* thread_message);

PN_CHAT_EXTERN PN_CHAT_EXPORT PnCResult pn_thread_message_parent_channel_id(Pubnub::ThreadMessage* thread_message, char* result);


#endif // PN_CHAT_C_THREAD_MESSAGE_H
