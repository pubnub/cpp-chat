#include <iostream>
#include "nlohmann/json.hpp"
#include "chat.hpp"

#include "chat/user.hpp"

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

void User::update(ChatUserData in_user_data)
{
    this->chat_obj
        .get_pubnub_context()
        .set_user_metadata(user_id, user_data_to_json(user_id, in_user_data));

    this->user_data = in_user_data;
}

void User::delete_user()
{
    this->chat_obj.delete_user(user_id);
}

void User::set_restrictions(String in_channel_id, bool ban_user, bool mute_user, String reason)
{
    this->chat_obj.set_restrictions(user_id, in_channel_id, ban_user, mute_user, reason);
}

void User::report(String reason)
{
    String payload = String("{") + reason + String(", reportedUserId: ") + user_id + String("}");
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

ChatUserData User::user_data_from_json(String data_json_string)
{
    json user_data_json = json::parse(data_json_string);

    if(user_data_json.is_null())
    {
        return ChatUserData();
    }

    ChatUserData user_data;

    if(user_data_json.contains("name") )
    {
        user_data.user_name = user_data_json["name"];
    }
    if(user_data_json.contains("externalId") )
    {
        user_data.external_id = user_data_json["externalId"];
    }
    if(user_data_json.contains("profileUrl") )
    {
        user_data.profile_url = user_data_json["profileUrl"];
    }
    if(user_data_json.contains("email") )
    {
        user_data.email = user_data_json["email"];
    }
    if(user_data_json.contains("custom") )
    {
        user_data.custom_data_json = user_data_json["custom"];
    }
    if(user_data_json.contains("status") )
    {
        user_data.status = user_data_json["status"];
    }
    if(user_data_json.contains("type") )
    {
        user_data.type = user_data_json["type"];
    }

    return user_data;
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

