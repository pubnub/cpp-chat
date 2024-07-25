#include "message_dao.hpp"
#include "message_action.hpp"
#include "domain/message_entity.hpp"

MessageDAO::MessageDAO(const MessageEntity& message_entity) :
    message_entity(std::move(message_entity))
{}

MessageDAO::MessageDAO(const Pubnub::ChatMessageData& message_data) :
    message_entity(entity_from_message_data(message_data))
{}

Pubnub::ChatMessageData MessageDAO::to_message_data() const {
    Pubnub::ChatMessageData message_data;

    message_data.type = this->message_entity.type;
    message_data.text = this->message_entity.text;
    message_data.channel_id = this->message_entity.channel_id;
    message_data.user_id = this->message_entity.user_id;
    message_data.meta = this->message_entity.meta;
    message_data.message_actions = std::move(this->message_entity.message_actions);

    return message_data;
}

MessageEntity MessageDAO::to_entity() const {
    return this->message_entity;
}

const MessageEntity& MessageDAO::get_entity() const {
    return this->message_entity;
}

MessageEntity MessageDAO::entity_from_message_data(Pubnub::ChatMessageData message_data) {
    MessageEntity message_entity;

    message_entity.type = message_data.type;
    message_entity.text = message_data.text;
    message_entity.channel_id = message_data.channel_id;
    message_entity.user_id = message_data.user_id;
    message_entity.meta = message_data.meta;
    message_entity.message_actions = message_data.message_actions.into_std_vector();

    return message_entity;
}
