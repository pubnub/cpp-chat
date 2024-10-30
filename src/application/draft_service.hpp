#ifndef PN_CHAT_DRAFT_SERVICE_HPP
#define PN_CHAT_DRAFT_SERVICE_HPP

#include "application/dao/message_draft_dao.hpp"
#include "string.hpp"

class DraftService {
    public:
        DraftService() = default;
        ~DraftService() = default;

        void insert_text_to_message(MessageDraftDAO& base, std::size_t position, const Pubnub::String& text_to_insert); 
        void remove_text_from_message(MessageDraftDAO& base, std::size_t position, std::size_t length);
};

#endif // PN_CHAT_DRAFT_SERVICE_HPP
