#include "chat/membership.hpp"
#include <iostream>
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

Membership::Membership(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, Pubnub::User in_user, Pubnub::String in_custom_json) :
    chat_obj(in_chat),
    channel(in_channel),
    user(in_user),
    custom_data_json(in_custom_json)
{}

void Pubnub::Membership::stream_updates(std::function<void(Membership)> membership_callback)
{
    std::vector<Pubnub::Membership> memberships;
    memberships.push_back(*this);
    stream_updates_on(memberships, membership_callback);
}

void Pubnub::Membership::stream_updates_on(std::vector<Pubnub::Membership> memberships, std::function<void(Membership)> membership_callback)
{
    if(memberships.empty())
    {
        throw std::invalid_argument("Cannot stream memberships updates on an empty list");
    }

    for(auto membership : memberships)
    {
        chat_obj.get_pubnub_context().register_membership_callback(channel.get_channel_id(), user.get_user_id(), membership_callback);
        chat_obj.get_pubnub_context().subscribe_to_channel(channel.get_channel_id());
    }
}

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