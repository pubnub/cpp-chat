#include "user_entity.hpp"
#include "domain/json.hpp"

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
        user_json.get_string("custom").value_or(Pubnub::String()),
        user_json.get_string("status").value_or(Pubnub::String()),
        user_json.get_string("type").value_or(Pubnub::String())
    };
}

