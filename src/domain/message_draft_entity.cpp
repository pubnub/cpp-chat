#include "message_draft_entity.hpp"

std::size_t MessageDraftMentionEntity::end_exclusive() const {
    return this->start + this->length;
}

MessageDraftEntity MessageDraftEntity::insert_text(std::size_t position, const Pubnub::String& text) const {
    auto new_value = this->value;
    auto new_mentions = this->mentions; 

    // TODO: mentions

    if (new_value.empty()) {
        new_value = text;
        return MessageDraftEntity{new_value, new_mentions};
    }

    new_value.replace(position, 0, text);
    
    return MessageDraftEntity{new_value, new_mentions};
}

MessageDraftEntity MessageDraftEntity::remove_text(std::size_t position, std::size_t length) const {
    auto new_value = this->value;
    auto new_mentions = this->mentions;

    // TODO: new_mentions

    new_value.erase(position, length);

    return MessageDraftEntity{new_value, new_mentions};
}
