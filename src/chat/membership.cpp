#include <iostream>
#include "chat/membership.hpp"
#include "nlohmann/json.hpp"
#include "string.hpp"
#include "chat/channel.hpp"
#include "chat/user.hpp"
#include "chat.hpp"

using namespace Pubnub;
using json = nlohmann::json;

Membership::Membership(Pubnub::Chat& in_chat, Pubnub::User in_user, Pubnub::String in_channel_json) :
    chat_obj(in_chat),
    user(in_user),
    channel(in_chat, in_channel_json)
{}

Membership::Membership(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, Pubnub::String in_member_json) :
    chat_obj(in_chat),
    channel(in_channel),
    user(in_chat, in_member_json)
{}
