#ifndef PN_CHAT_MESSAGE_DRAFT_DAO_HPP
#define PN_CHAT_MESSAGE_DRAFT_DAO_HPP

#include "domain/message_draft_entity.hpp"

class MessageDraftDAO {
    public:
        MessageDraftDAO() = default;
        MessageDraftDAO(const MessageDraftEntity& entity);
        ~MessageDraftDAO() = default;

        MessageDraftEntity get_entity() const;

    private:
        const MessageDraftEntity entity;
};

#endif // PN_CHAT_MESSAGE_DRAFT_DAO_HPP
