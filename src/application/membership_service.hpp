#ifndef PN_CHAT_MEMBERSHIP_SERVICE_HPP
#define PN_CHAT_MEMBERSHIP_SERVICE_HPP

#include "domain/membership_entity.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include "membership.hpp"
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

        Pubnub::String get_membership_custom_data(const Pubnub::String& user_id, const Pubnub::String& channel_id) const;
        std::vector<Pubnub::Membership> get_channel_members(const Pubnub::String& channel_id, int limit, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken) const;
        std::vector<Pubnub::Membership> get_user_memberships(const Pubnub::String& user_id, int limit, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken) const;
        Pubnub::Membership invite_to_channel(const Pubnub::String& channel_id, const Pubnub::User& user) const;
        std::vector<Pubnub::Membership> invite_multiple_to_channel(const Pubnub::String& channel_id, const std::vector<Pubnub::User>& users) const;
    
        Pubnub::Membership update(const Pubnub::User& user, const Pubnub::Channel& channel, const Pubnub::String& custom_object_json) const;
        Pubnub::String last_read_message_timetoken(const Pubnub::Membership& membership) const;
        void set_last_read_message_timetoken(const Pubnub::Membership& membership, const Pubnub::String& timetoken) const;
        int get_unread_messages_count_one_channel(const Pubnub::Membership& membership) const;
        std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> get_all_unread_messages_counts(const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, const Pubnub::String& filter = "", int limit = 0) const;


        void stream_updates_on(const std::vector<Pubnub::Membership>& memberships, std::function<void(const Pubnub::Membership&)> membership_callback) const;

        Pubnub::Membership create_presentation_object(const Pubnub::User& user, const Pubnub::Channel& channel) const;
        Pubnub::Membership create_membership_object(const Pubnub::User& user, const Pubnub::Channel& channel, const MembershipEntity& membership_entity) const;


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        MembershipEntity create_domain_membership(const Pubnub::String& custom_object_json) const;

        friend class ::ChannelService;

};

#endif // PN_CHAT_MEMBERSHIP_SERVICE_HPP
