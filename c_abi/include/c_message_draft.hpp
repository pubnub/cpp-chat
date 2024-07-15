#ifndef PN_CHAT_C_MESSAGE_DRAFT_H
#define PN_CHAT_C_MESSAGE_DRAFT_H

#include "message_draft.hpp"
#include "user.hpp"
#include "helpers/export.hpp"
#include "helpers/extern.hpp"

PN_CHAT_EXTERN PN_CHAT_EXPORT void pn_message_draft_delete(Pubnub::MessageDraft* message_draft);

#endif // PN_CHAT_C_MESSAGE_DRAFT_H
