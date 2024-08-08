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

Restriction User::get_channel_restrictions(const Pubnub::Channel& channel) const {
    return this->restrictions_service->get_channel_restrictions(user_id(), channel.channel_id());
}

void User::report(const String& reason) const {
    this->restrictions_service->report_user(user_id(), reason);
}

MembershipsResponseWrapper User::get_memberships(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    auto return_tuple = this->membership_service->get_user_memberships(user_id(), *this->data, filter, sort, limit, page);
    return MembershipsResponseWrapper({Pubnub::Vector<Membership>(std::move(std::get<0>(return_tuple))), std::get<1>(return_tuple), std::get<2>(return_tuple), std::get<3>(return_tuple)});
}

CallbackStop User::stream_updates(std::function<void(const User&)> user_callback) const {
    return CallbackStop(this->user_service->stream_updates(*this, user_callback));
}

CallbackStop User::stream_updates_on(Pubnub::Vector<Pubnub::User> users, std::function<void(Pubnub::Vector<Pubnub::User>)> user_callback) const {
    auto users_std = users.into_std_vector();

    auto new_callback = [=](std::vector<Pubnub::User> vec)
    {
        user_callback(std::move(vec));
    };
    return CallbackStop(this->user_service->stream_updates_on(*this, users_std, new_callback));
}
