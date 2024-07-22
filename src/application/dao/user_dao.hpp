#ifndef PN_CHAT_USER_DAO_HPP
#define PN_CHAT_USER_DAO_HPP

#include "domain/user_entity.hpp"
#include "user.hpp"
class UserDAO {
    public:
        UserDAO() = default;
        UserDAO(const UserEntity user_entity);
        UserDAO(const Pubnub::ChatUserData& user_data);
        ~UserDAO() = default;

        Pubnub::ChatUserData to_user_data() const;

        UserEntity to_entity() const;
        const UserEntity& get_entity() const;

    private:
        static UserEntity entity_from_user_data(const Pubnub::ChatUserData& user_data);

        const UserEntity user_entity;
 };

#endif // PN_CHAT_USER_DAO_HPP
