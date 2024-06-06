#include <iostream>
#include "chat/membership.hpp"
#include "nlohmann/json.hpp"
#include "string.hpp"
#include "chat/channel.hpp"
#include "chat/user.hpp"

using namespace Pubnub;
using json = nlohmann::json;

Memebership::Memebership(Pubnub::Chat& in_chat, Pubnub::User& in_user, Pubnub::String in_membership_json) :
    chat_obj(in_chat),
    user(in_user),
    channel(Channel(in_chat, in_membership_json))
{}

Memebership::Memebership(Pubnub::Chat& in_chat, Pubnub::Channel& in_channel, Pubnub::String in_member_json) :
    chat_obj(in_chat),
    channel(in_channel),
    user(User(in_chat, in_member_json))
{}
