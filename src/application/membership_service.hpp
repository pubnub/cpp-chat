#ifndef PN_CHAT_MEMBERSHIP_SERVICE_HPP
#define PN_CHAT_MEMBERSHIP_SERVICE_HPP

#include "infra/sync.hpp"
#include "string.hpp"
#include "presentation/membership.hpp"
#include <memory>
#include <vector>
#include <functional>

class EntityRepository;
class PubNub;
class ChatService;


class MembershipService : public std::enable_shared_from_this<MembershipService>
{
    public:
        MembershipService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);

        std::vector<Pubnub::Membership> get_channel_members(Pubnub::String channel_id, int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);
        Pubnub::Membership invite_to_channel(Pubnub::String channel_id, Pubnub::User user);
        std::vector<Pubnub::Membership> invite_multiple_to_channel(Pubnub::String channel_id, std::vector<Pubnub::User> users);
    
    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        Pubnub::Membership create_presentation_object(Pubnub::User user, Pubnub::Channel channel);

};

#endif // PN_CHAT_MEMBERSHIP_SERVICE_HPP
