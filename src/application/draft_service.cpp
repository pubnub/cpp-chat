#include "draft_service.hpp"

void DraftService::insert_text_to_message(MessageDraftDAO& dao, std::size_t position, const Pubnub::String& text_to_insert) {
    dao.update_entity(dao.get_entity().insert_text(position, text_to_insert));
}

void DraftService::remove_text_from_message(MessageDraftDAO& dao, std::size_t position, std::size_t length) {
    dao.update_entity(dao.get_entity().remove_text(position, length));
}


