#ifndef PN_CHAT_C_MESSAGE_DRAFT_H
#define PN_CHAT_C_MESSAGE_DRAFT_H

#include "message_draft.hpp"
#include "user.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_delete(Pubnub::MessageDraft* message_draft);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_insert_text(Pubnub::MessageDraft* message_draft, std::size_t position, const char* text_to_insert);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_remove_text(Pubnub::MessageDraft* message_draft, std::size_t position, std::size_t length);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_add_mention(Pubnub::MessageDraft* message_draft, std::size_t start, std::size_t length, const char* mention_json);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_remove_mention(Pubnub::MessageDraft* message_draft, std::size_t start);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_update(Pubnub::MessageDraft* message_draft, const char* text);

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_send(Pubnub::MessageDraft* message_draft, const Pubnub::SendTextParams* send_params);

#endif // PN_CHAT_C_MESSAGE_DRAFT_H
