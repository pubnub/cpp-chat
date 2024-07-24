#include "membership.hpp"
#include "message.hpp"
#include "application/membership_service.hpp"
#include "application/dao/membership_dao.hpp"

using namespace Pubnub;

Membership::Membership(User user, Channel channel, std::shared_ptr<const ChatService> chat_service, std::shared_ptr<const MembershipService> membership_service, std::unique_ptr<MembershipDAO> data) :
user(user),
channel(channel),
chat_service(chat_service),
membership_service(membership_service),
data(std::move(data))
{}

Membership::Membership(const Membership& other) :
user(other.user),
channel(other.channel),
chat_service(other.chat_service),
membership_service(other.membership_service),
data(std::make_unique<MembershipDAO>(other.data->to_custom_data()))
{}

Membership& Membership::operator =(const Membership& other)
{
    if(this == &other)
    {
        return *this;
    }
    this->user = other.user;
    this->channel = other.channel;
    this->data = std::make_unique<::MembershipDAO>(other.data->to_custom_data());
    this->chat_service = other.chat_service;
    this->membership_service = other.membership_service;

    return *this;
};

Membership::~Membership() = default;

String Membership::custom_data() const {
    return this->data->to_custom_data();
}

Membership Membership::update(const String& custom_object_json) const {
    return this->membership_service->update(user, channel, custom_object_json);
}

String Membership::last_read_message_timetoken() const {
    return this->membership_service->last_read_message_timetoken(*this);
}

Membership Membership::set_last_read_message_timetoken(const String& timetoken) const {
    return this->membership_service->set_last_read_message_timetoken(*this, timetoken);
}

Membership Membership::set_last_read_message(const Message& message) const {
    return this->membership_service->set_last_read_message_timetoken(*this, message.timetoken());
}

int Membership::get_unread_messages_count(const Membership& membership) const {
    return this->membership_service->get_unread_messages_count_one_channel(*this);
}

void Membership::stream_updates(std::function<void(const Membership&)> membership_callback) const {
    this->membership_service->stream_updates_on({*this}, membership_callback);
}

void Membership::stream_updates_on(const std::vector<Membership>& memberships, std::function<void(const Membership&)> membership_callback) const {
    this->membership_service->stream_updates_on(memberships, membership_callback);
}
