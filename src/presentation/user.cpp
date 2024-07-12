#include "user.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/membership_service.hpp"

using namespace Pubnub;

User::User(Pubnub::String user_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<UserService> user_service, std::shared_ptr<PresenceService> presence_service, 
            std::shared_ptr<RestrictionsService> restrictions_service, std::shared_ptr<MembershipService> membership_service) :
user_id_internal(user_id),
chat_service(chat_service),
user_service(user_service),
presence_service(presence_service),
restrictions_service(restrictions_service),
membership_service(membership_service)
{}

ChatUserData User::user_data()
{
    return user_service->get_user_data(user_id_internal);
}

User User::update(ChatUserData user_data)
{
    return this->user_service->update_user(user_id_internal, user_data);
}

void User::delete_user()
{
    this->user_service->delete_user(user_id_internal);
}

std::vector<Pubnub::String> User::where_present()
{
    return this->presence_service->where_present(user_id_internal);
}

bool User::is_present_on(Pubnub::String channel_id)
{
    return this->presence_service->is_present(user_id_internal, channel_id);
}

void User::set_restrictions(String channel_id, Restriction restrictions)
{
    this->restrictions_service->set_restrictions(user_id_internal, channel_id, restrictions);
}

Restriction User::get_channel_restrictions(Pubnub::String user_id, Pubnub::String channel_id, int limit, String start, String end)
{
    return this->restrictions_service->get_channel_restrictions(user_id, channel_id, limit, start, end);
}

void User::report(String reason)
{
    this->restrictions_service->report_user(user_id(), reason);
}

std::vector<Pubnub::Membership> User::get_memberships(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken)
{
    return this->membership_service->get_user_memberships(user_id(), limit, start_timetoken, end_timetoken);
}

void User::stream_updates(std::function<void(User)> user_callback)
{
    this->user_service->stream_updates_on({*this}, user_callback);
}
void User::stream_updates_on(std::vector<Pubnub::User> users, std::function<void(User)> user_callback)
{
    this->user_service->stream_updates_on(users, user_callback);
}
