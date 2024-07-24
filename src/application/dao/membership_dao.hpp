#ifndef PN_CHAT_MEMBERSHIP_DAO_HPP
#define PN_CHAT_MEMBERSHIP_DAO_HPP

#include "domain/membership_entity.hpp"
#include "string.hpp"

class MembershipDAO {
    public:
        MembershipDAO(const Pubnub::String& custom_data);
        MembershipDAO(const MembershipEntity& entity);
        ~MembershipDAO() = default;

        Pubnub::String to_custom_data() const;
        const MembershipEntity& get_entity() const;
        MembershipEntity to_entity() const;

    private:
        MembershipEntity entity;
};

#endif // PN_CHAT_MEMBERSHIP_DAO_HPP
