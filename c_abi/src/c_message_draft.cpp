#include "c_message_draft.hpp"
#include "c_errors.hpp"

void pn_message_draft_delete(Pubnub::MessageDraft* message_draft) {
    delete message_draft;
}