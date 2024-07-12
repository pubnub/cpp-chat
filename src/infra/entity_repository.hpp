#ifndef PN_CHAT_ENTITY_REPOSITORY_HPP
#define PN_CHAT_ENTITY_REPOSITORY_HPP

#include "domain/membership_entity.hpp"
#include "domain/user_entity.hpp"
#include "domain/channel_entity.hpp"
#include "domain/message_entity.hpp"
#include "repository.hpp"
#include "string.hpp"

class EntityRepository {
    using ChannelId = Pubnub::String;
    using UserId = Pubnub::String;

    public: 
        EntityRepository();

        Repository<Pubnub::String, UserEntity, Pubnub::StringComparer>& get_user_entities();
        Repository<Pubnub::String, ChannelEntity, Pubnub::StringComparer>& get_channel_entities();
        Repository<Pubnub::String, MessageEntity, Pubnub::StringComparer>& get_message_entities();
        Repository<std::pair<UserId, ChannelId>, MembershipEntity>& get_membership_entities();

     private:
        Repository<Pubnub::String, UserEntity, Pubnub::StringComparer> user_entities;
        Repository<Pubnub::String, ChannelEntity, Pubnub::StringComparer> channel_entities;
        Repository<Pubnub::String, MessageEntity, Pubnub::StringComparer> message_entities;
        Repository<std::pair<UserId, ChannelId>, MembershipEntity> membership_entities;
};

#endif // PN_CHAT_ENTITY_REPOSITORY_HPP
