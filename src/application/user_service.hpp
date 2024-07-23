#ifndef PN_CHAT_USER_SERVICE_HPP
#define PN_CHAT_USER_SERVICE_HPP

#include "user.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include <memory>
#include <vector>
#include <functional>
#include "application/dao/user_dao.hpp"

class EntityRepository;
class PubNub;
class ChatService;
struct UserEntity;

class UserService : public std::enable_shared_from_this<UserService>
{
    public:
        UserService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service);

        Pubnub::User get_current_user() const;

        Pubnub::User create_user(const Pubnub::String& user_id, const UserDAO& user_data) const;
        Pubnub::User get_user(const Pubnub::String& user_id) const;
        std::vector<Pubnub::User> get_users(const Pubnub::String& include, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
        Pubnub::User update_user(const Pubnub::String& user_id, const UserDAO& user_data) const;
        void delete_user(const Pubnub::String& user_id) const;
        void stream_updates_on(const std::vector<Pubnub::User>& users, std::function<void(const Pubnub::User&)> user_callback) const;

        Pubnub::User create_user_object(std::pair<Pubnub::String, UserDAO> user_data) const;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;

        friend class ::MembershipService;
};

#endif // PN_CHAT_USER_SERVICE_HPP
