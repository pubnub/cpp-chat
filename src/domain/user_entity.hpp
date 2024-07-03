#ifndef PN_CHAT_USER_ENTITY_HPP
#define PN_CHAT_USER_ENTITY_HPP

#include "string.hpp"

struct UserEntity {
    Pubnub::String user_id = "";
    Pubnub::String user_name = "";
    Pubnub::String external_id = "";
    Pubnub::String profile_url = "";
    Pubnub::String email = "";
    Pubnub::String custom_data_json = "";
    Pubnub::String status = "";
    Pubnub::String type = "";

    Pubnub::String get_user_metadata_json_string();
};

#endif // PN_CHAT_USER_ENTITY_HPP
