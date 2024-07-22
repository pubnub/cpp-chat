#include "user_dao.hpp"

UserDAO::UserDAO(const UserEntity user_entity) :
    user_entity(std::move(user_entity))
{}

UserDAO::UserDAO(const Pubnub::ChatUserData& user_data) :
    user_entity(entity_from_user_data(user_data))
{}

Pubnub::ChatUserData UserDAO::to_user_data() const {
    Pubnub::ChatUserData user_data;

    user_data.user_name = this->user_entity.user_name;
    user_data.external_id = this->user_entity.external_id;
    user_data.profile_url = this->user_entity.profile_url;
    user_data.email = this->user_entity.email;
    user_data.custom_data_json = this->user_entity.custom_data_json;
    user_data.status = this->user_entity.status;
    user_data.type = this->user_entity.type;

    return user_data;
}

UserEntity UserDAO::to_entity() const {
    return this->user_entity;
}

const UserEntity& UserDAO::get_entity() const {
    return this->user_entity;
}
