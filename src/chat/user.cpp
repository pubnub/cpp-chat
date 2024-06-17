#include "chat/user.hpp"
#include <iostream>
#include "nlohmann/json.hpp"
#include "chat.hpp"


#include "chat/membership.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

User::User(Chat& chat, String user_id, ChatUserData additional_user_data) :
    chat_obj(chat),
    user_id(user_id),
    user_data(additional_user_data)
{
    //now channel is fully initialized
    this->is_initialized = true;
}

User::User(Chat& chat, String user_id, String user_data_json) :
    User(chat, user_id, user_data_from_json(user_data_json)) {}

User::User(Pubnub::Chat& chat, String user_data_json) :
    User(chat, user_id_from_json(user_data_json), user_data_from_json(user_data_json)) {}

User User::update(ChatUserData in_user_data)
{
    this->chat_obj
        .get_pubnub_context()
        .set_user_metadata(user_id, user_data_to_json(user_id, in_user_data));

    this->user_data = in_user_data;

    return *this;
}

void User::delete_user()
{
    this->chat_obj.delete_user(user_id);
}

void User::set_restrictions(String in_channel_id, PubnubRestrictionsData restrictions)
{
    this->chat_obj.set_restrictions(user_id, in_channel_id, restrictions);
}

void User::report(String reason)
{
    String payload = String("{\"reason\": \"") + reason + String("\", \"reportedUserId\": \"") + user_id + String("\"}");
    this->chat_obj
        .emit_chat_event(pubnub_chat_event_type::PCET_REPORT, this->chat_obj.internal_admin_channel, payload);
}

std::vector<Pubnub::String> User::where_present()
{
    return this->chat_obj.where_present(user_id);
}

bool User::is_present_on(Pubnub::String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get is present on, channel_id is empty");
    }

    return this->chat_obj.is_present(user_id, channel_id);
}

std::vector<Pubnub::Membership> User::get_memberships(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields,channelTypeField,statusField,channelStatusField";
    String get_memberships_response = chat_obj.get_pubnub_context().get_memberships(user_id, include_string, limit, start_timetoken, end_timetoken);

    json response_json = json::parse(get_memberships_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get memberships, response is incorrect");
    }

    json channels_array_json = response_json["data"];


    std::vector<Membership> memberships;

    for (auto& element : channels_array_json)
    {
        Membership membership_obj(chat_obj, *this, String(element["channel"].dump()));
        memberships.push_back(membership_obj);
    }

    return memberships;
}

PubnubRestrictionsData User::get_channel_restrictions(Pubnub::String in_user_id, Pubnub::String in_channel_id, int limit, String start, String end)
{
    String full_channel_id = chat_obj.internal_moderation_prefix + in_channel_id;

    String get_restrictions_response = chat_obj.get_pubnub_context().get_channel_members(full_channel_id, "totalCount,custom", limit, start, end);

    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channel restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    PubnubRestrictionsData FinalRestrictionsData;

   for (auto& element : response_data_json)
   {
        //Find restrictions data for requested channel
        if(String(element["uuid"]["id"]) == in_user_id)
        {
            if(element["custom"]["ban"] == true)
            {
                FinalRestrictionsData.ban = true;
            }
            if(element["custom"]["mute"] == true)
            {
                FinalRestrictionsData.mute = true;
            }
            FinalRestrictionsData.reason = String(element["custom"]["reason"]);
            break;
        }
   }

   return FinalRestrictionsData;
}

void User::stream_updates(std::function<void(User)> user_callback)
{
    std::vector<Pubnub::User> users;
    users.push_back(*this);
    stream_updates_on(users, user_callback);
}
void User::stream_updates_on(std::vector<Pubnub::User> users, std::function<void(User)> user_callback)
{
    if(users.empty())
    {
        throw std::invalid_argument("Cannot stream user updates on an empty list");
    }

    for(auto user : users)
    {
        chat_obj.get_pubnub_context().register_user_callback(user.user_id, user_callback);
        chat_obj.get_pubnub_context().subscribe_to_channel(user.user_id);
    }
}
ChatUserData User::user_data_from_json(String data_json_string)
{
    json user_data_json = json::parse(data_json_string);

    if(user_data_json.is_null())
    {
        return ChatUserData();
    }

    ChatUserData user_data;

    if(user_data_json.contains("name") && !user_data_json["name"].is_null())
    {
        user_data.user_name = user_data_json["name"].dump();
    }
    if(user_data_json.contains("externalId") && !user_data_json["externalId"].is_null())
    {
        user_data.external_id = user_data_json["externalId"].dump();
    }
    if(user_data_json.contains("profileUrl") && !user_data_json["profileUrl"].is_null())
    {
        user_data.profile_url = user_data_json["profileUrl"].dump();
    }
    if(user_data_json.contains("email") && !user_data_json["email"].is_null())
    {
        user_data.email = user_data_json["email"].dump();
    }
    if(user_data_json.contains("custom") && !user_data_json["custom"].is_null())
    {
        user_data.custom_data_json = user_data_json["custom"];
    }
    if(user_data_json.contains("status") && !user_data_json["status"].is_null())
    {
        user_data.status = user_data_json["status"].dump();
    }
    if(user_data_json.contains("type") && !user_data_json["type"].is_null())
    {
        user_data.type = user_data_json["type"].dump();
    }

    return user_data;
}

String User::user_id_from_json(String data_json_string)
{
    json user_data_json = json::parse(data_json_string);

    if(user_data_json.is_null())
    {
        return "";
    }

    ChatUserData user_data;

    if(user_data_json.contains("id") )
    {
        auto dumped = user_data_json["id"].dump();

        // removal of quotes
        return String(&dumped[1], dumped.size() - 2);
    }

    return "";
}

String User::user_data_to_json(String in_user_id, ChatUserData in_user_data)
{
    json user_data_json;

    user_data_json["id"] = in_user_id.c_str();

    if(!in_user_data.user_name.empty())
    {
        user_data_json["name"] = in_user_data.user_name.c_str();
    }
    if(!in_user_data.external_id.empty())
    {
        user_data_json["externalId"] = in_user_data.external_id.c_str();
    }
    if(!in_user_data.profile_url.empty())
    {
        user_data_json["profileUrl"] = in_user_data.profile_url.c_str();
    }
    if(!in_user_data.email.empty())
    {
        user_data_json["email"] = in_user_data.email.c_str();
    }
    if(!in_user_data.custom_data_json.empty())
    {
        user_data_json["custom"] = in_user_data.custom_data_json.c_str();
    }
    if(!in_user_data.status.empty())
    {
        user_data_json["status"] = in_user_data.status.c_str();
    }
    if(!in_user_data.type.empty())
    {
        user_data_json["type"] = in_user_data.type.c_str();
    }

    return user_data_json.dump();
}

Pubnub::String User::get_user_id()
{
    return user_id;
}

ChatUserData User::get_user_data()
{
    return user_data;
}
