#ifndef PN_CHAT_MEMBERSHIP_SERVICE_HPP
#define PN_CHAT_MEMBERSHIP_SERVICE_HPP

#include "domain/membership_entity.hpp"
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

        Pubnub::String get_membership_custom_data(Pubnub::String user_id, Pubnub::String channel_id);
        std::vector<Pubnub::Membership> get_channel_members(Pubnub::String channel_id, int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);
        std::vector<Pubnub::Membership> get_user_memberships(Pubnub::String user_id, int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);
        Pubnub::Membership invite_to_channel(Pubnub::String channel_id, Pubnub::User user);
        std::vector<Pubnub::Membership> invite_multiple_to_channel(Pubnub::String channel_id, std::vector<Pubnub::User> users);
    
        Pubnub::Membership update(Pubnub::User user, Pubnub::Channel channel, Pubnub::String custom_object_json);
        Pubnub::String last_read_message_timetoken(Pubnub::Membership membership);
        void set_last_read_message_timetoken(Pubnub::Membership membership, Pubnub::String timetoken);
        int get_unread_messages_count_one_channel(Pubnub::Membership membership);
        std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> get_all_unread_messages_counts(Pubnub::String start_timetoken, Pubnub::String end_timetoken, Pubnub::String filter = "", int limit = 0);


        void stream_updates_on(std::vector<Pubnub::Membership> memberships, std::function<void(Pubnub::Membership)> membership_callback);

        Pubnub::Membership create_presentation_object(Pubnub::User user, Pubnub::Channel channel);
        Pubnub::Membership create_membership_object(Pubnub::User user, Pubnub::Channel channel, MembershipEntity membership_entity);


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        MembershipEntity create_domain_membership(Pubnub::String custom_object_json);

        friend class ::ChannelService;

};

#endif // PN_CHAT_MEMBERSHIP_SERVICE_HPP
