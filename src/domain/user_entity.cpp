#include "user_entity.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Pubnub::String UserEntity::get_user_metadata_json_string()
{
    json user_data_json;

    user_data_json["id"] = user_id.c_str();

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

