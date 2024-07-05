#ifndef PN_CHAT_C_MESSAGE_DRAFT_H
#define PN_CHAT_C_MESSAGE_DRAFT_H

#include "chat/message_draft.hpp"
#include "chat/user.hpp"
#include "export.hpp"
#include "extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_delete(Pubnub::MessageDraft* message_draft);

#endif // PN_CHAT_C_MESSAGE_DRAFT_H
