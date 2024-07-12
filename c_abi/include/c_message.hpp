#ifndef PN_CHAT_C_MESSAGE_H
#define PN_CHAT_C_MESSAGE_H

#include "chat/message.hpp"
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

#endif // PN_CHAT_C_MESSAGE_H
