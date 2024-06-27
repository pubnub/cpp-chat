#include "user_entity.hpp"

UserEntity::UserEntity(Pubnub::String user_id, UserData user_data):
    user_id(user_id),
    user_data(user_data)
{}
