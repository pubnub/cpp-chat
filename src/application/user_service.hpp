#ifndef PN_CHAT_USER_SERVICE_HPP
#define PN_CHAT_USER_SERVICE_HPP

#include "presentation/user.hpp"
#include "export.hpp"
#include "infra/sync.hpp"
#include <memory>

class EntityRepository;
class PubNub;
class UserEntity;

class UserService : public std::enable_shared_from_this<UserService>
{
    public:
        PN_CHAT_EXPORT UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;


        UserEntity create_domain_from_presentation_data(Pubnub::String user_id, Pubnub::ChatUserData& presentation_data);
};

#endif // PN_CHAT_USER_SERVICE_HPP
