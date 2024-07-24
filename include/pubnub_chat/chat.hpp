#ifndef PN_CHAT_CHAT_HPP
#define PN_CHAT_CHAT_HPP

#include "string.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "user.hpp"
#include "membership.hpp"
#include "restrictions.hpp"
#include "vector.hpp"
#include <memory>
#include <vector>
#include <functional>

class ChatService;
class ChannelService;
class UserService;
class PresenceService;
class RestrictionsService;
class MessageService;
class MembershipService;

#ifndef PN_CHAT_C_ABI
class CallbackService;
#endif

namespace Pubnub {

    struct CreatedChannelWrapper
    {
        Pubnub::Channel created_channel;
        Pubnub::Membership host_membership;
        std::vector<Pubnub::Membership> invitees_memberships;

        CreatedChannelWrapper(Pubnub::Channel in_channel, Pubnub::Membership in_host_membership, std::vector<Pubnub::Membership> in_invitees_memberships) :
        created_channel(in_channel),
        host_membership(in_host_membership),
        invitees_memberships(in_invitees_memberships)
        {}

        CreatedChannelWrapper(Pubnub::Channel in_channel, Pubnub::Membership in_host_membership) :
        created_channel(in_channel),
        host_membership(in_host_membership)
        {}
    };

    struct UnreadMessageWrapper
    {
        Pubnub::Channel channel;
        Pubnub::Membership membership;
        int count;
    };


    class Chat {
        public:
            PN_CHAT_EXPORT Chat(String publish_key, String subscribe_key, String user_id);

            PN_CHAT_EXPORT Pubnub::Vector<int> TestVector();
            PN_CHAT_EXPORT Pubnub::Vector<int> TestVector2(int param);
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::String> TestVectorString();
            /* CHANNELS */

            PN_CHAT_EXPORT Pubnub::Channel create_public_conversation(const Pubnub::String& channel_id, const ChatChannelData& channel_data) const;
            PN_CHAT_EXPORT CreatedChannelWrapper create_direct_conversation(const Pubnub::User& user, const Pubnub::String& channel_id, const ChatChannelData& channel_data, const Pubnub::String& membership_data = "") const;
            PN_CHAT_EXPORT CreatedChannelWrapper create_group_conversation(const std::vector<Pubnub::User>& users, const Pubnub::String& channel_id, const ChatChannelData& channel_data, const Pubnub::String& membership_data = "") const;
            PN_CHAT_EXPORT Channel get_channel(const Pubnub::String& channel_id) const;
            PN_CHAT_EXPORT Pubnub::Vector<Channel> get_channels(const Pubnub::String& include, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
            PN_CHAT_EXPORT Pubnub::Channel update_channel(const Pubnub::String& channel_id, const ChatChannelData& channel_data) const;
            PN_CHAT_EXPORT void delete_channel(const Pubnub::String& channel_id) const;
            PN_CHAT_EXPORT void pin_message_to_channel(const Pubnub::Message& message, const Pubnub::Channel& channel) const;
            PN_CHAT_EXPORT void unpin_message_from_channel(const Pubnub::Channel& channel) const;

            /* USERS */

            PN_CHAT_EXPORT Pubnub::User create_user(const Pubnub::String& user_id, const Pubnub::ChatUserData& user_data) const;
            PN_CHAT_EXPORT Pubnub::User get_user(const Pubnub::String& user_id) const;
            PN_CHAT_EXPORT std::vector<User> get_users(const Pubnub::String& include, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
            PN_CHAT_EXPORT Pubnub::User update_user(const Pubnub::String& user_id, const Pubnub::ChatUserData& user_data) const;
            PN_CHAT_EXPORT void delete_user(const Pubnub::String& user_id) const;

            /* PRESENCE */

            PN_CHAT_EXPORT std::vector<Pubnub::String> where_present(const Pubnub::String& user_id) const;
            PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present(const Pubnub::String& channel_id) const;
            PN_CHAT_EXPORT bool is_present(const Pubnub::String& user_id, const Pubnub::String& channel_id) const;

            /* MODERATION */

            PN_CHAT_EXPORT void set_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, const Pubnub::Restriction& restrictions) const;
            PN_CHAT_EXPORT void emit_chat_event(pubnub_chat_event_type chat_event_type, const Pubnub::String& channel_id, const Pubnub::String& payload) const;
            PN_CHAT_EXPORT void listen_for_events(const Pubnub::String& channel_id, pubnub_chat_event_type chat_event_type, std::function<void(const Pubnub::String&)> event_callback) const;

            /* MESSAGES */

            PN_CHAT_EXPORT void forward_message(const Pubnub::Message& message, const Pubnub::Channel& channel) const;
            PN_CHAT_EXPORT std::vector<Pubnub::UnreadMessageWrapper> get_unread_message_counts(const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, const Pubnub::String& filter = "", int limit = 0);

        private:
            std::shared_ptr<const ChatService> chat_service;
            std::shared_ptr<const ChannelService> channel_service;
            std::shared_ptr<const UserService> user_service;
            std::shared_ptr<const PresenceService> presence_service;
            std::shared_ptr<const RestrictionsService> restrictions_service;
            std::shared_ptr<const MessageService> message_service;
            std::shared_ptr<const MembershipService> membership_service;
#ifndef PN_CHAT_C_ABI
            std::shared_ptr<CallbackService> callback_service;
#else
        public:
            ChatService* get_chat_service();
#endif

    };
}

#endif // PN_CHAT_CHAT_HPP
