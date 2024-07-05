#ifndef PN_CHAT_MEMBERSHIP_SERVICE_HPP
#define PN_CHAT_MEMBERSHIP_SERVICE_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class ChatService;


class MembershipService : public std::enable_shared_from_this<MembershipService>
{
    public:
        MembershipService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

};

#endif // PN_CHAT_MEMBERSHIP_SERVICE_HPP
