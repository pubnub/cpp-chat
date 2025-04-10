#ifndef PN_CHAT_MEMBERSHIP_DAO_HPP
#define PN_CHAT_MEMBERSHIP_DAO_HPP

#include "domain/membership_entity.hpp"
#include "string.hpp"

class MembershipDAO {
    public:
        MembershipDAO(const Pubnub::ChatMembershipData& membership_data);
        MembershipDAO(const MembershipEntity& entity);
        ~MembershipDAO() = default;

        Pubnub::ChatMembershipData to_membership_data() const;
        const MembershipEntity& get_entity() const;
        MembershipEntity to_entity() const;

    private:
        static MembershipEntity entity_from_membership_data(const Pubnub::ChatMembershipData& membership_data);

        MembershipEntity entity;
};

#endif // PN_CHAT_MEMBERSHIP_DAO_HPP
