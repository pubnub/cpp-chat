#include "message_draft_dao.hpp"
#include "domain/message_draft_entity.hpp"

MessageDraftEntity MessageDraftDAO::get_entity() const {
    return this->entity;
}
