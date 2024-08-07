#ifndef PN_CHAT_MEMBERSHIP_ENTITY_HPP
#define PN_CHAT_MEMBERSHIP_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"
#include "membership.hpp"

struct MembershipEntity {
    Pubnub::String custom_field;

    static MembershipEntity from_json(Json membership_json);

    static MembershipEntity from_base_and_updated_membership(MembershipEntity base_membership, MembershipEntity updated_membership);
};

#endif // PN_CHAT_MEMBERSHIP_ENTITY_HPP

