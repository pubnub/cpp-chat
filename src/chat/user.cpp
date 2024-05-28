#include <iostream>
#include "nlohmann/json.hpp"
#include "chat.hpp"

#include "chat/user.hpp"

extern "C" {
#include "core/pubnub_objects_api.h"
}

using namespace Pubnub;
using json = nlohmann::json;

void User::init(Chat *InChat, String in_user_id, ChatUserData in_additional_user_data)
{
    chat_obj = InChat;
    user_id = in_user_id;
    user_data = in_additional_user_data;

    //now channel is fully initialized
    is_initialized = true;
}

void User::init_from_json(Chat *InChat, String in_user_id, String user_data_json)
{
    init(InChat, in_user_id, user_data_from_json(user_data_json));
}

void User::update(ChatUserData in_user_data)
{
    user_data = in_user_data;
    pubnub_set_uuidmetadata(get_ctx_pub(), user_id, NULL, user_data_to_json(user_id, user_data));
}

void User::delete_user()
{
    if(!chat_obj)
    {
        throw std::runtime_error("Failed to delete user, chat_obj is invalid");
    }

    chat_obj->delete_user(user_id);
}

void User::set_restrictions(String in_channel_id, bool ban_user, bool mute_user, String reason)
{
    if(!chat_obj)
    {
        throw std::invalid_argument("Failed to set restrictions, chat_obj is invalid");
    }

    chat_obj->set_restrictions(user_id, in_channel_id, ban_user, mute_user, reason);
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

    user_data_json["id"] = in_user_id;

    if(in_user_data.user_name.empty())
    {
        user_data_json["name"] = in_user_data.user_name;
    }
    if(in_user_data.external_id.empty())
    {
        user_data_json["externalId"] = in_user_data.external_id;
    }
    if(in_user_data.profile_url.empty())
    {
        user_data_json["profileUrl"] = in_user_data.profile_url;
    }
    if(in_user_data.email.empty())
    {
        user_data_json["email"] = in_user_data.email;
    }
    if(in_user_data.custom_data_json.empty())
    {
        user_data_json["custom"] = in_user_data.custom_data_json;
    }
    if(in_user_data.status != 0 )
    {
        user_data_json["status"] = in_user_data.status;
    }
    if(in_user_data.type.empty())
    {
        user_data_json["type"] = in_user_data.type;
    }

    return user_data_json.dump();
}

pubnub_t *User::get_ctx_pub()
{
    if(!chat_obj)
    //TODO: throw exception, error or sth
    {return nullptr;}
    return chat_obj->get_pubnub_context();
}