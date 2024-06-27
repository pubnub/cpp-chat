#ifndef PN_CHAT_ENTITY_REPOSITORY_HPP
#define PN_CHAT_ENTITY_REPOSITORY_HPP

#include "domain/user_entity.hpp"
#include "domain/channel_entity.hpp"
#include "domain/chat_entity.hpp"
#include "domain/message_entity.hpp"
#include "sync.hpp"
#include "repository.hpp"

class EntityRepository {
    public: 
        EntityRepository();
        void update_or_insert_user_entity(UserEntity user_entity);
        void update_or_insert_channel_entity(ChannelEntity channel_entity);
        void update_or_insert_message_entity(MessageEntity message_entity);

        UserEntity get_user_entity(Pubnub::String user_id);
        ChannelEntity get_channel_entity(Pubnub::String channel_id);
        MessageEntity get_message_entity(Pubnub::String timetoken);

        void delete_user_entity(Pubnub::String user_id);
        void delete_channel_entity(Pubnub::String channel_id);
        void delete_message_entity(Pubnub::String timetoken);

        bool user_entity_exists(Pubnub::String user_id);
        bool channel_entity_exists(Pubnub::String channel_id);
        bool message_entity_exists(Pubnub::String timetoken);
    private:
        Repository<Pubnub::String, UserEntity> user_entities;
        Repository<Pubnub::String, ChannelEntity> channel_entities;
        Repository<Pubnub::String, MessageEntity> message_entities;
};

#endif // PN_CHAT_ENTITY_REPOSITORY_HPP
