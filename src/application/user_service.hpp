#ifndef PN_CHAT_USER_SERVICE_HPP
#define PN_CHAT_USER_SERVICE_HPP

#include "application/subscription.hpp"
#include "infra/interval_task.hpp"
#include "infra/timer.hpp"
#include "user.hpp"
#include "infra/sync.hpp"
#include "string.hpp"
#include "page.hpp"
#include <memory>
#include <optional>
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
        UserService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service, int store_user_active_interval);

        Pubnub::User get_current_user() const;

        Pubnub::User create_user(const Pubnub::String& user_id, const UserDAO& user_data, bool skip_get_user = false) const;
        Pubnub::User get_user(const Pubnub::String& user_id) const;
        std::tuple<std::vector<Pubnub::User>, Pubnub::Page, int> get_users(const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page& page = Pubnub::Page()) const;
        Pubnub::User update_user(const Pubnub::String& user_id, const UserDAO& user_data) const;
        void delete_user(const Pubnub::String& user_id) const;
        std::shared_ptr<Subscription> stream_updates(Pubnub::User calling_user, std::function<void(const Pubnub::User)> user_callback) const;
        std::shared_ptr<SubscriptionSet> stream_updates_on(Pubnub::User calling_user, const std::vector<Pubnub::User>& users, std::function<void(std::vector<Pubnub::User>)> user_callback) const;
        std::vector<Pubnub::User> get_users_suggestions(Pubnub::String text, int limit = 10) const;

        Pubnub::User create_user_object(std::pair<Pubnub::String, UserDAO> user_data) const;
        Pubnub::User update_user_with_base(const Pubnub::User& user, const Pubnub::User& base_user) const;

        bool active(const UserDAO& user_data) const;
        Pubnub::Option<Pubnub::String> last_active_timestamp(const UserDAO& user_data) const;

        void store_user_activity_timestamp() const;

    private:
        void run_save_timestamp_interval(UserEntity user_entity) const;
        void save_timestamp_function(UserEntity user_entity) const;

        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;
        int store_user_active_interval;
        mutable std::optional<IntervalTask> lastSavedActivityInterval;
        mutable Timer save_timestamp_timer;

        friend class ::MembershipService;

};

#endif // PN_CHAT_USER_SERVICE_HPP
