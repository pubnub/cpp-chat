#include "presentation/membership.hpp"
#include "application/membership_service.hpp"

using namespace Pubnub;

Membership::Membership(User user, Channel channel, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service) :
user(user),
channel(channel),
chat_service(chat_service),
membership_service(membership_service)
{}

Membership Membership::update(String custom_object_json)
{
    return this->membership_service->update(user, channel, custom_object_json);
}

void Membership::stream_updates(std::function<void(Membership)> membership_callback)
{
    this->membership_service->stream_updates_on({*this}, membership_callback);
}

void Membership::stream_updates_on(std::vector<Membership> memberships, std::function<void(Membership)> membership_callback)
{
    this->membership_service->stream_updates_on(memberships, membership_callback);
}