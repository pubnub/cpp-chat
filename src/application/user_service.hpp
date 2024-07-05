#ifndef PN_CHAT_USER_SERVICE_HPP
#define PN_CHAT_USER_SERVICE_HPP

#include "presentation/user.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class ChatService;
struct UserEntity;

class UserService : public std::enable_shared_from_this<UserService>
{
    public:
        UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        Pubnub::User create_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
        Pubnub::User get_user(Pubnub::String user_id);
        std::vector<Pubnub::User> get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        Pubnub::User update_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
        void delete_user(Pubnub::String user_id);

        Pubnub::User create_presentation_object(Pubnub::String user_id);
        Pubnub::User create_user_object(std::pair<Pubnub::String, UserEntity> user_data);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        UserEntity create_domain_from_presentation_data(Pubnub::String user_id, Pubnub::ChatUserData& presentation_data);
        //Creates ChannelEntity from user response - put the whole response, not only "data" field
        UserEntity create_domain_from_user_response(Pubnub::String json_response);
        //Creates ChannelEntity from user response "data" field
        UserEntity create_domain_from_user_response_data(Pubnub::String json_response_data);

        Pubnub::ChatUserData presentation_data_from_domain(UserEntity& user_entity);

        friend class ::MembershipService;
};

#endif // PN_CHAT_USER_SERVICE_HPP
