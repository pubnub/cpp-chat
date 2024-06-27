#ifndef PN_CHAT_ENTITY_REPOSITORY_HPP
#define PN_CHAT_ENTITY_REPOSITORY_HPP

#include "domain/user_entity.hpp"
#include "domain/channel_entity.hpp"
#include "domain/message_entity.hpp"
#include "repository.hpp"

class EntityRepository {
    public: 
        EntityRepository();

        Repository<Pubnub::String, UserEntity>& get_user_entities();
        Repository<Pubnub::String, ChannelEntity>& get_channel_entities();
        Repository<Pubnub::String, MessageEntity>& get_message_entities();
     private:
        Repository<Pubnub::String, UserEntity> user_entities;
        Repository<Pubnub::String, ChannelEntity> channel_entities;
        Repository<Pubnub::String, MessageEntity> message_entities;
};

#endif // PN_CHAT_ENTITY_REPOSITORY_HPP
