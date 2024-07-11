#include "membership_entity.hpp"

MembershipEntity MembershipEntity::from_json(Json membership_json) {
    return MembershipEntity{
        membership_json.get_string("custom").value_or(Pubnub::String())
    };
}
