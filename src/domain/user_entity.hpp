#ifndef PN_CHAT_USER_ENTITY_HPP
#define PN_CHAT_USER_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include <optional>
#include <vector>

struct UserEntity {
    using UserId = Pubnub::String;

    Pubnub::String user_name = Pubnub::String("");
    Pubnub::String external_id = Pubnub::String("");
    Pubnub::String profile_url = Pubnub::String("");
    Pubnub::String email = Pubnub::String("");
    Pubnub::String custom_data_json = Pubnub::String("");
    Pubnub::String status = Pubnub::String("");
    Pubnub::String type = Pubnub::String("");

    Pubnub::String get_user_metadata_json_string(Pubnub::String user_id);

    std::optional<Pubnub::String> get_last_active_timestamp() const;
    void set_last_active_timestamp(Pubnub::String timestamp);
    bool is_active(int activity_interval) const;

    static UserEntity from_json(Json user_json);
    static UserEntity from_user_response(Json response);
    static std::vector<std::pair<UserId, UserEntity>> from_user_list_response(Json response);

    static UserEntity from_base_and_updated_user(UserEntity base_user, UserEntity updated_user);
};

#endif // PN_CHAT_USER_ENTITY_HPP
