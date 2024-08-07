#include "membership_entity.hpp"

MembershipEntity MembershipEntity::from_json(Json membership_json) {
    return MembershipEntity{
        membership_json.contains("custom") ? membership_json["custom"].dump() : Pubnub::String("")
    };
}

MembershipEntity MembershipEntity::from_base_and_updated_membership(MembershipEntity base_membership, MembershipEntity updated_membership)
{
    MembershipEntity new_entity;
    new_entity.custom_field = updated_membership.custom_field.empty() ? base_membership.custom_field : updated_membership.custom_field;
    return new_entity;
}