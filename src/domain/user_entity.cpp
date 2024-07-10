#include "user_entity.hpp"
#include "domain/json.hpp"

Pubnub::String UserEntity::get_user_metadata_json_string()
{
    Json user_data_json;

    if(!user_name.empty())
    {
        user_data_json["name"] = user_name.c_str();
    }
    if(!external_id.empty())
    {
        user_data_json["externalId"] = user_name.c_str();
    }
    if(!profile_url.empty())
    {
        user_data_json["profileUrl"] = profile_url.c_str();
    }
    if(!email.empty())
    {
        user_data_json["email"] = email.c_str();
    }
    if(!custom_data_json.empty())
    {
        user_data_json["custom"] = custom_data_json.c_str();
    }
    if(!status.empty())
    {
        user_data_json["status"] = status.c_str();
    }
    if(!type.empty())
    {
        user_data_json["type"] = type.c_str();
    }

    return user_data_json.dump();
}

UserEntity UserEntity::from_json(Json user_json) {
    return UserEntity{
        user_json.get_string("name").value_or(Pubnub::String()),
        user_json.get_string("externalId").value_or(Pubnub::String()),
        user_json.get_string("profileUrl").value_or(Pubnub::String()),
        user_json.get_string("email").value_or(Pubnub::String()),
        user_json.get_string("custom").value_or(Pubnub::String()),
        user_json.get_string("status").value_or(Pubnub::String()),
        user_json.get_string("type").value_or(Pubnub::String())
    };
}

