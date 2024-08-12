#include "user_entity.hpp"
#include "domain/json.hpp"
#include <utility>
#include <vector>

Pubnub::String UserEntity::get_user_metadata_json_string(Pubnub::String user_id)
{
    Json user_data_json;

    user_data_json.insert_or_update("id", user_id);
    if(!user_name.empty()) {
        user_data_json.insert_or_update("name", user_name);
    }
    if(!external_id.empty()) {
        user_data_json.insert_or_update("externalId", external_id);
    }
    if(!profile_url.empty()) {
        user_data_json.insert_or_update("profileUrl", profile_url);
    }
    if(!email.empty()) {
        user_data_json.insert_or_update("email", email);
    }
    if(!custom_data_json.empty()) {
        Json custom_json = Json::parse(custom_data_json);
        user_data_json.insert_or_update("custom", custom_json);
    }
    if(!status.empty()) {
        user_data_json.insert_or_update("status", status);
    }
    if(!type.empty()) {
        user_data_json.insert_or_update("type", type);
    }

    return user_data_json.dump();
}

UserEntity UserEntity::from_json(Json user_json) {
    UserEntity new_entity;
    if(user_json.contains("uuid") && !user_json["uuid"].is_null())
    {
        new_entity.user_name = user_json["uuid"].get_string("name").value_or(Pubnub::String());
        new_entity.external_id = user_json["uuid"].get_string("externalId").value_or(Pubnub::String());
        new_entity.profile_url = user_json["uuid"].get_string("profileUrl").value_or(Pubnub::String());
        new_entity.email = user_json["uuid"].get_string("email").value_or(Pubnub::String());
    }
    else
    {
        new_entity.user_name = user_json.get_string("name").value_or(Pubnub::String());
        new_entity.external_id = user_json.get_string("externalId").value_or(Pubnub::String());
        new_entity.profile_url = user_json.get_string("profileUrl").value_or(Pubnub::String());
        new_entity.email = user_json.get_string("email").value_or(Pubnub::String());
    
    }
    new_entity.custom_data_json = user_json.contains("custom") ? user_json["custom"].dump() : Pubnub::String();
    new_entity.status = user_json.get_string("status").value_or(Pubnub::String()),
    new_entity.type = user_json.get_string("type").value_or(Pubnub::String());
    
    return new_entity;
}

UserEntity UserEntity::from_user_response(Json response) {
    return UserEntity{
        response.get_string("name").value_or(Pubnub::String()),
        response.get_string("externalId").value_or(Pubnub::String()),
        response.get_string("profileUrl").value_or(Pubnub::String()),
        response.get_string("email").value_or(Pubnub::String()),
        response.contains("custom") ? response["custom"].dump() : Pubnub::String(),
        response.get_string("status").value_or(Pubnub::String()),
        response.get_string("type").value_or(Pubnub::String())
    };
}

std::vector<std::pair<UserEntity::UserId, UserEntity>> UserEntity::from_user_list_response(Json response) {
    std::vector<std::pair<UserEntity::UserId, UserEntity>> users;

    for(auto user : response["data"]) {
        users.push_back(std::make_pair(user["id"], UserEntity::from_json(user.dump())));
    }

    return users;

}

UserEntity UserEntity::from_base_and_updated_user(UserEntity base_user, UserEntity updated_user)
{
    UserEntity new_entity;
    new_entity.user_name = updated_user.user_name.empty() ? base_user.user_name : updated_user.user_name;
    new_entity.external_id = updated_user.external_id.empty() ? base_user.external_id : updated_user.external_id;
    new_entity.profile_url = updated_user.profile_url.empty() ? base_user.profile_url : updated_user.profile_url;
    new_entity.email = updated_user.email.empty() ? base_user.email : updated_user.email;
    new_entity.custom_data_json = updated_user.custom_data_json.empty() ? base_user.custom_data_json : updated_user.custom_data_json;
    new_entity.status = updated_user.status.empty() ? base_user.status : updated_user.status;
    new_entity.type = updated_user.type.empty() ? base_user.type : updated_user.type;
    return new_entity;
}