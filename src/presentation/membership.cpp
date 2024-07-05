#include "presentation/membership.hpp"

using namespace Pubnub;

Membership::Membership(Pubnub::User user, Pubnub::Channel channel, Pubnub::String custom_data_json, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MembershipService> membership_service) :
user(user),
channel(channel),
custom_data_json(custom_data_json),
chat_service(chat_service),
membership_service(membership_service)
{}