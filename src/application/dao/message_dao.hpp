#ifndef PN_CHAT_MESSAGE_DAO_HPP
#define PN_CHAT_MESSAGE_DAO_HPP

#include "domain/message_entity.hpp"
#include "chat_message_data.hpp"

class MessageDAO {
    public:
        MessageDAO(const Pubnub::ChatMessageData& message_data);
        MessageDAO(const MessageEntity& message_entity);
        ~MessageDAO() = default;

        Pubnub::ChatMessageData to_message_data() const;
        MessageEntity to_entity() const;
        const MessageEntity& get_entity() const;

    private:
        static MessageEntity entity_from_message_data(Pubnub::ChatMessageData message_data);

        const MessageEntity message_entity;
};

#endif // PN_CHAT_MESSAGE_DAO_HPP

