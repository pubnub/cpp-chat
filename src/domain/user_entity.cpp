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
    return UserEntity{
        user_json.get_string("name").value_or(Pubnub::String()),
        user_json.get_string("externalId").value_or(Pubnub::String()),
        user_json.get_string("profileUrl").value_or(Pubnub::String()),
        user_json.get_string("email").value_or(Pubnub::String()),
        user_json.contains("custom") ? user_json["custom"].dump() : Pubnub::String(),
        user_json.get_string("status").value_or(Pubnub::String()),
        user_json.get_string("type").value_or(Pubnub::String())
    };
}

UserEntity UserEntity::from_user_response(Json response) {
    //In most responses this data field is an array but in some cases (for example in get_user) it's just an object.
    Json user_data_json = response["data"].is_array() ? response["data"][0] : response["data"];

    return UserEntity::from_json(user_data_json);
}

std::vector<std::pair<UserEntity::UserId, UserEntity>> UserEntity::from_user_list_response(Json response) {
    std::vector<std::pair<UserEntity::UserId, UserEntity>> users;

    for(auto user : response["data"]) {
        users.push_back(std::make_pair(user["id"], UserEntity::from_json(user.dump())));
    }

    return users;

}
