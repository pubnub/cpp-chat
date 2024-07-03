#ifndef PN_CHAT_USER_SERVICE_HPP
#define PN_CHAT_USER_SERVICE_HPP

#include "presentation/user.hpp"
#include "export.hpp"
#include "infra/sync.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class UserEntity;

class UserService : public std::enable_shared_from_this<UserService>
{
    public:
        PN_CHAT_EXPORT UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);

        PN_CHAT_EXPORT Pubnub::User create_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
        PN_CHAT_EXPORT Pubnub::User get_user(Pubnub::String user_id);
        PN_CHAT_EXPORT std::vector<Pubnub::User> get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        PN_CHAT_EXPORT Pubnub::User update_user(Pubnub::String user_id, Pubnub::ChatUserData user_data);
        PN_CHAT_EXPORT void delete_user(Pubnub::String user_id);
        std::vector<Pubnub::String> where_present(Pubnub::String user_id);


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;


        UserEntity create_domain_from_presentation_data(Pubnub::String user_id, Pubnub::ChatUserData& presentation_data);
        //Creates ChannelEntity from user response - put the whole response, not only "data" field
        UserEntity create_domain_from_user_response(Pubnub::String json_response);
        //Creates ChannelEntity from user response "data" field
        UserEntity create_domain_from_user_response_data(Pubnub::String json_response_data);

        Pubnub::ChatUserData presentation_data_from_domain(UserEntity& user_entity);
};

#endif // PN_CHAT_USER_SERVICE_HPP
