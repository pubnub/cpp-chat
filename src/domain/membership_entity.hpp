#ifndef PN_CHAT_MEMBERSHIP_ENTITY_HPP
#define PN_CHAT_MEMBERSHIP_ENTITY_HPP

#include "domain/json.hpp"
#include "string.hpp"

struct MembershipEntity {
    Pubnub::String custom_field;

    static MembershipEntity from_json(Json membership_json);
};

#endif // PN_CHAT_MEMBERSHIP_ENTITY_HPP

