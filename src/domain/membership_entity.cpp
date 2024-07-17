#include "membership_entity.hpp"

MembershipEntity MembershipEntity::from_json(Json membership_json) {
    return MembershipEntity{
        membership_json.contains("custom") ? membership_json["custom"].dump() : Pubnub::String()
    };
}
