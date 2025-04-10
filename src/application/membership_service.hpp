#ifndef PN_CHAT_MEMBERSHIP_SERVICE_HPP
#define PN_CHAT_MEMBERSHIP_SERVICE_HPP

#include "application/dao/channel_dao.hpp"
#include "application/dao/user_dao.hpp"
#include "application/subscription.hpp"
#include "domain/membership_entity.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include "membership.hpp"
#include "page.hpp"
#include <memory>
#include <vector>
#include <functional>

class EntityRepository;
class PubNub;
class ChatService;


class MembershipService : public std::enable_shared_from_this<MembershipService>
{
    public:
        MembershipService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<const ChatService> chat_service);

        std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> get_channel_members(const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page& page = Pubnub::Page()) const;
        std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> get_user_memberships(const Pubnub::String& user_id, const UserDAO& user_data, const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page& page = Pubnub::Page()) const;
        Pubnub::Membership invite_to_channel(const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::User& user) const;
        std::vector<Pubnub::Membership> invite_multiple_to_channel(const Pubnub::String& channel_id, const ChannelDAO& channel_data, const std::vector<Pubnub::User>& users) const;
    
        Pubnub::Membership update(const Pubnub::User& user, const Pubnub::Channel& channel, MembershipDAO membership_data) const;
        Pubnub::String last_read_message_timetoken(const Pubnub::Membership& membership) const;
        Pubnub::Membership set_last_read_message_timetoken(const Pubnub::Membership& membership, const Pubnub::String& timetoken) const;
        int get_unread_messages_count_one_channel(const Pubnub::Membership& membership) const;
        std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> get_all_unread_messages_counts(const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page& page = Pubnub::Page()) const;
        std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>> mark_all_messages_as_read(const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page &page = Pubnub::Page()) const;

        std::shared_ptr<Subscription> stream_updates(Pubnub::Membership calling_membership, std::function<void(Pubnub::Membership)> membership_callback) const;
        std::shared_ptr<SubscriptionSet> stream_updates_on(Pubnub::Membership calling_membership, const std::vector<Pubnub::Membership>& memberships, std::function<void(std::vector<Pubnub::Membership>)> membership_callback) const;

        Pubnub::Membership create_membership_object(const Pubnub::User& user, const Pubnub::Channel& channel) const;
        Pubnub::Membership create_membership_object(const Pubnub::User& user, const Pubnub::Channel& channel, const MembershipEntity& membership_entity) const;

        Pubnub::Membership update_membership_with_base(const Pubnub::Membership& membership, const Pubnub::Membership& base_membership) const;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;

        MembershipEntity create_domain_membership(const Pubnub::String& custom_object_json) const;

        friend class ::ChannelService;

        //Message counts requires a valid timetoken, so this one will be like "0", from beginning of the channel
        Pubnub::String EMPTY_TIMETOKEN = "17000000000000000";

#ifdef PN_CHAT_C_ABI
    public:
        void stream_updates_on(const std::vector<Pubnub::String>& membership_ids) const;
        void stream_read_receipts(const Pubnub::String& membership_id, const MembershipDAO& membership_data) const;
#endif
};

#endif // PN_CHAT_MEMBERSHIP_SERVICE_HPP
