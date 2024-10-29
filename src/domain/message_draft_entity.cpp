#include "message_draft_entity.hpp"

std::size_t MessageDraftMentionEntity::end_exclusive() const {
    return this->start + this->length;
}

MessageDraftEntity MessageDraftEntity::insert_text(std::size_t position, const Pubnub::String& text) const {
    auto new_value = this->value;
    auto new_mentions = this->mentions; 

    // TODO: mentions

    new_value.replace(position, 0, text);
    
    return MessageDraftEntity{new_value, new_mentions};
}
