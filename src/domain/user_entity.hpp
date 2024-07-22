#ifndef PN_CHAT_USER_ENTITY_HPP
#define PN_CHAT_USER_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include <vector>

struct UserEntity {
    using UserId = Pubnub::String;

    Pubnub::String user_name;
    Pubnub::String external_id;
    Pubnub::String profile_url;
    Pubnub::String email;
    Pubnub::String custom_data_json;
    Pubnub::String status;
    Pubnub::String type;

    Pubnub::String get_user_metadata_json_string(Pubnub::String user_id);

    static UserEntity from_json(Json user_json);
    static UserEntity from_user_response(Json response);
    static std::vector<std::pair<UserId, UserEntity>> from_user_list_response(Json response);
};

#endif // PN_CHAT_USER_ENTITY_HPP
