#include "draft_service.hpp"

MessageDraftDAO DraftService::insert_text_to_message(const MessageDraftDAO& base, std::size_t position, const Pubnub::String& text_to_insert) {
    MessageDraftDAO result = base;
    result.get_entity().insert_text(position, text_to_insert);
    return result;
}

MessageDraftDAO DraftService::remove_text_from_message(const MessageDraftDAO& base, std::size_t position, std::size_t length) {
    MessageDraftDAO result = base;
    return result;
}
