#ifndef PN_CHAT_USER_H
#define PN_CHAT_USER_H

#include "string.hpp"
#include "helpers/export.hpp"
#include "restrictions.hpp"
#include "vector.hpp"
#include <memory>
#include <vector>
#include <functional>

class UserService;
class ChatService;
class PresenceService;
class RestrictionsService;
class MembershipService;
class UserDAO;

namespace Pubnub
{
    class Membership;

    struct ChatUserData
    {
        Pubnub::String user_name = "";
        Pubnub::String external_id = "";
        Pubnub::String profile_url = "";
        Pubnub::String email = "";
        Pubnub::String custom_data_json = "";
        Pubnub::String status = "";
        Pubnub::String type = "";
    };

    class User
    {
        public:
            PN_CHAT_EXPORT User(const User& other);
            PN_CHAT_EXPORT ~User();

            PN_CHAT_EXPORT Pubnub::User& operator =(const Pubnub::User& other);

            PN_CHAT_EXPORT Pubnub::String user_id() const;
            PN_CHAT_EXPORT Pubnub::ChatUserData user_data() const;

            PN_CHAT_EXPORT Pubnub::User update(const Pubnub::ChatUserData& user_data) const;
            PN_CHAT_EXPORT void delete_user() const;
            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::String> where_present() const;
            PN_CHAT_EXPORT bool is_present_on(const Pubnub::String& channel_id) const;

            PN_CHAT_EXPORT void set_restrictions(const Pubnub::String& channel_id, const Pubnub::Restriction& restrictions) const;
            PN_CHAT_EXPORT Pubnub::Restriction get_channel_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
            PN_CHAT_EXPORT void report(const Pubnub::String& reason) const;

            PN_CHAT_EXPORT Pubnub::Vector<Pubnub::Membership> get_memberships(int limit, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken) const;

            PN_CHAT_EXPORT void stream_updates(std::function<void(const User&)> user_callback) const;
            PN_CHAT_EXPORT void stream_updates_on(Pubnub::Vector<Pubnub::User> users, std::function<void(const Pubnub::User&)> user_callback) const;

        private:
            PN_CHAT_EXPORT User(
                    Pubnub::String user_id,
                    std::shared_ptr<const ChatService> chat_service,
                    std::shared_ptr<const UserService> user_service,
                    std::shared_ptr<const PresenceService> presence_service,
                    std::shared_ptr<const RestrictionsService> restrictions_service,
                    std::shared_ptr<const MembershipService> membership_service,
                    std::unique_ptr<UserDAO> data);
            
            Pubnub::String user_id_internal;
            std::unique_ptr<UserDAO> data;
            std::shared_ptr<const UserService> user_service;
            std::shared_ptr<const ChatService> chat_service;
            std::shared_ptr<const PresenceService> presence_service;
            std::shared_ptr<const RestrictionsService> restrictions_service;
            std::shared_ptr<const MembershipService> membership_service;

        friend class ::UserService;
    };
}
#endif /* PN_CHAT_USER_H */