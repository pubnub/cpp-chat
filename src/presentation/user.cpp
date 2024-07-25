#include "user.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/membership_service.hpp"
#include "application/dao/user_dao.hpp" 

using namespace Pubnub;

User::User(Pubnub::String user_id, std::shared_ptr<const ChatService> chat_service, std::shared_ptr<const UserService> user_service, std::shared_ptr<const PresenceService> presence_service, std::shared_ptr<const RestrictionsService> restrictions_service, std::shared_ptr<const MembershipService> membership_service, std::unique_ptr<UserDAO> data) :
user_id_internal(user_id),
chat_service(chat_service),
user_service(user_service),
presence_service(presence_service),
restrictions_service(restrictions_service),
membership_service(membership_service),
data(std::move(data))
{}

User::User(const User& other) :
user_id_internal(other.user_id_internal),
chat_service(other.chat_service),
user_service(other.user_service),
presence_service(other.presence_service),
restrictions_service(other.restrictions_service),
membership_service(other.membership_service),
data(std::make_unique<UserDAO>(other.data->to_user_data()))
{}

User& User::operator =(const User& other)
{
    if(this == &other)
    {
        return *this;
    }
    this->user_id_internal = other.user_id_internal;
    this->data = std::make_unique<::UserDAO>(other.data->to_user_data());
    this->user_service = other.user_service;
    this->chat_service = other.chat_service;
    this->presence_service = other.presence_service;
    this->restrictions_service = other.restrictions_service;
    this->membership_service = other.membership_service;

    return *this;
};

User::~User() = default;

Pubnub::String User::user_id() const {
    return user_id_internal;
}

ChatUserData User::user_data() const {
    return this->data->to_user_data();
}

User User::update(const ChatUserData& user_data) const {
    return this->user_service->update_user(user_id_internal, user_data);
}

void User::delete_user() const {
    this->user_service->delete_user(user_id_internal);
}

Pubnub::Vector<Pubnub::String> User::where_present() const {
    return Pubnub::Vector<String>(std::move(this->presence_service->where_present(user_id_internal)));
}

bool User::is_present_on(const Pubnub::String& channel_id) const {
    return this->presence_service->is_present(user_id_internal, channel_id);
}

void User::set_restrictions(const String& channel_id, const Restriction& restrictions) const {
    this->restrictions_service->set_restrictions(user_id_internal, channel_id, restrictions);
}

Restriction User::get_channel_restrictions(const Pubnub::String& user_id, const Pubnub::String& channel_id, int limit, const String& start, const String& end) const {
    return this->restrictions_service->get_channel_restrictions(user_id, channel_id, limit, start, end);
}

void User::report(const String& reason) const {
    this->restrictions_service->report_user(user_id(), reason);
}

Pubnub::Vector<Pubnub::Membership> User::get_memberships(int limit, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken) const {
    return Pubnub::Vector<Membership>(std::move(this->membership_service->get_user_memberships(user_id(), *this->data, limit, start_timetoken, end_timetoken)));
}

void User::stream_updates(std::function<void(const User&)> user_callback) const {
    this->user_service->stream_updates_on({*this}, user_callback);
}

void User::stream_updates_on(Pubnub::Vector<Pubnub::User> users, std::function<void(const User&)> user_callback) const {
    this->user_service->stream_updates_on(users.into_std_vector(), user_callback);
}
