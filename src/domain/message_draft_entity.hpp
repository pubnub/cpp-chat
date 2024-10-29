#ifndef PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP
#define PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP

#include "string.hpp"

struct MessageDraftMentionTargetEntity {
    Pubnub::String target;
    Pubnub::String type;
};

struct MessageDraftMentionEntity {
    std::size_t start;
    std::size_t length;
    MessageDraftMentionTargetEntity target;

    std::size_t end_exclusive() const;
    
};

struct MessageDraftEntity {
    Pubnub::String value = "";
    std::vector<MessageDraftMentionEntity> mentions = {};

    MessageDraftEntity insert_text(std::size_t position, const Pubnub::String& text_to_insert) const; 
};

#endif // PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP
