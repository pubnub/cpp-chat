#ifndef PN_CHAT_DRAFT_SERVICE_HPP
#define PN_CHAT_DRAFT_SERVICE_HPP

#include "application/dao/message_draft_dao.hpp"
#include "message_draft.hpp"
#include "string.hpp"

class DraftService {
    public:
        DraftService() = default;
        ~DraftService() = default;

        void insert_text_to_message(MessageDraftDAO& dao, std::size_t position, const Pubnub::String& text_to_insert); 
        void remove_text_from_message(MessageDraftDAO& dao, std::size_t position, std::size_t length);
        void insert_suggested_mention_to_message(MessageDraftDAO& dao, const Pubnub::SuggestedMention suggested_mention, const Pubnub::String& text);
        void add_mention_to_message(MessageDraftDAO& dao, std::size_t position, std::size_t length, const Pubnub::MentionTarget& target);
        void remove_mention_from_message(MessageDraftDAO& dao, std::size_t position);
        void update_message(MessageDraftDAO& dao, const Pubnub::String& text);
};

#endif // PN_CHAT_DRAFT_SERVICE_HPP
