#include "presentation/membership.hpp"

using namespace Pubnub;

Membership::Membership(Pubnub::User user, Pubnub::Channel channel, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service) :
user(user),
channel(channel),
chat_service(chat_service),
membership_service(membership_service)
{}
