#include "entity_repository.hpp"

EntityRepository::EntityRepository():
    user_entities(),
    channel_entities(),
    message_entities()
{}

Repository<Pubnub::String, UserEntity>& EntityRepository::get_user_entities() {
    return user_entities;
}

Repository<Pubnub::String, ChannelEntity>& EntityRepository::get_channel_entities() {
    return channel_entities;
}

Repository<Pubnub::String, MessageEntity>& EntityRepository::get_message_entities() {
    return message_entities;
}
