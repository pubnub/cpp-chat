#include "message_entity.hpp"

MessageEntity::MessageEntity(Pubnub::String timetoken, MessageData message_data):
    timetoken(timetoken),
    message_data(message_data)
{}
