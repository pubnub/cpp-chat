#include "membership.hpp"
#include "message.hpp"
#include "application/membership_service.hpp"

using namespace Pubnub;

Membership::Membership(User user, Channel channel, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service) :
user(user),
channel(channel),
chat_service(chat_service),
membership_service(membership_service)
{}

String Membership::custom_data()
{
    return this->membership_service->get_membership_custom_data(user.user_id(), channel.channel_id());
}

Membership Membership::update(String custom_object_json)
{
    return this->membership_service->update(user, channel, custom_object_json);
}

String Membership::last_read_message_timetoken()
{
    return this->membership_service->last_read_message_timetoken(*this);
}

Membership Membership::set_last_read_message_timetoken(String timetoken)
{
    this->membership_service->set_last_read_message_timetoken(*this, timetoken);
    return *this;
}

Membership Membership::set_last_read_message(Message message)
{
    this->membership_service->set_last_read_message_timetoken(*this, message.timetoken());
    return *this;
}

int Membership::get_unread_messages_count(Pubnub::Membership membership)
{
    return this->membership_service->get_unread_messages_count_one_channel(*this);
}

void Membership::stream_updates(std::function<void(Membership)> membership_callback)
{
    this->membership_service->stream_updates_on({*this}, membership_callback);
}

void Membership::stream_updates_on(std::vector<Membership> memberships, std::function<void(Membership)> membership_callback)
{
    this->membership_service->stream_updates_on(memberships, membership_callback);
}
