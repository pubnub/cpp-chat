#include "membership_entity.hpp"
#include "domain/json.hpp"

Pubnub::String MembershipEntity::get_set_memberships_json_string(Pubnub::String channel_id){
    Json channel_json;
    channel_json.insert_or_update("id", channel_id);

    Json set_memberships_json;

    set_memberships_json.insert_or_update("channel", channel_json);

    if(!custom_field.empty()) {
        Json custom_json = Json::parse(custom_field);
        set_memberships_json.insert_or_update("custom", custom_json);
    }
    else 
    {
        set_memberships_json.insert_or_update("custom", "{}");
    }
    if(!status.empty()) {
        set_memberships_json.insert_or_update("status", status);
    }
    if(!type.empty()) {
        set_memberships_json.insert_or_update("type", type);
    }

    return Pubnub::String("[" + set_memberships_json.dump() + "]");
}

MembershipEntity MembershipEntity::from_json(Json membership_json) {
    return MembershipEntity{
        membership_json.contains("custom") ? membership_json["custom"].dump() : Pubnub::String(""),
        membership_json.get_string("status").value_or(Pubnub::String("")),
        membership_json.get_string("type").value_or(Pubnub::String(""))
    };
}

MembershipEntity MembershipEntity::from_base_and_updated_membership(MembershipEntity base_membership, MembershipEntity updated_membership)
{
    MembershipEntity new_entity;
    new_entity.custom_field = updated_membership.custom_field.empty() ? base_membership.custom_field : updated_membership.custom_field;
    new_entity.status = updated_membership.status.empty() ? base_membership.status : updated_membership.status;
    new_entity.type = updated_membership.type.empty() ? base_membership.type : updated_membership.type;
    return new_entity;
}