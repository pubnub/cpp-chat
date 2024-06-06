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


Membership Membership::update(Pubnub::String in_custom_object_json)
{
    if(in_custom_object_json.empty())
    {
        throw std::invalid_argument("Failed to update membership. in_custom_object_json is empty");
    }
    json response_json = json::parse(in_custom_object_json);
    
    if(response_json.is_null() || !response_json.is_object())
    {
        throw std::invalid_argument("Can't update membership, in_custom_object_json is not valid json object");
    }

	String set_memberships_string = String("[{\"channel\": {\"id\": \"") + channel.get_channel_id() + String("\"}, \"custom\": ") + in_custom_object_json + String("}]");
    chat_obj.get_pubnub_context().set_memberships(user.get_user_id(), set_memberships_string);

    custom_data_json = in_custom_object_json;
    return *this;
}