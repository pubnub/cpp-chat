#ifndef PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP
#define PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP

#include "string.hpp"

struct MessageDraftMentionTargetEntity {
    enum class Type {
        USER,
        CHANNEL, 
        URL
    };

    Pubnub::String target;
    Type type;
};

struct MessageDraftMentionEntity {
    std::size_t start;
    std::size_t length;
    MessageDraftMentionTargetEntity target;

    std::size_t end_exclusive() const;
    
};

struct MessageDraftSuggestedMentionEntity {
    std::size_t position;
    Pubnub::String replace_from;
    Pubnub::String replace_to;
    MessageDraftMentionTargetEntity target;
};

struct MessageDraftEntity {
    Pubnub::String value = "";
    std::vector<MessageDraftMentionEntity> mentions = {};

    MessageDraftEntity insert_text(std::size_t position, const Pubnub::String& text_to_insert) const; 
    MessageDraftEntity remove_text(std::size_t position, std::size_t length) const;

    MessageDraftEntity add_mention(std::size_t start, std::size_t length, const MessageDraftMentionTargetEntity& target) const;
    MessageDraftEntity remove_mention(std::size_t start) const;

    MessageDraftEntity insert_suggested_mention(const MessageDraftSuggestedMentionEntity& suggested_mention, const Pubnub::String& text) const;

    bool validate_mentions() const;
    bool validate_suggested_mention(const MessageDraftSuggestedMentionEntity& suggested_mention) const;
};

#endif // PN_CHAT_MESSAGE_DRAFT_ENTITY_HPP
