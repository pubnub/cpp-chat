#include "membership_dao.hpp"
#include "domain/membership_entity.hpp"

MembershipDAO::MembershipDAO(const Pubnub::ChatMembershipData& membership_data) :
    entity(entity_from_membership_data(membership_data))
{}

MembershipDAO::MembershipDAO(const MembershipEntity& entity) :
    entity(entity)
{}

MembershipEntity MembershipDAO::to_entity() const {
    return this->entity;
}

const MembershipEntity& MembershipDAO::get_entity() const {
    return this->entity;
}

Pubnub::ChatMembershipData MembershipDAO::to_membership_data() const {
    Pubnub::ChatMembershipData membership_data;
    membership_data.custom_data_json = this->entity.custom_field;
    membership_data.status = this->entity.status;
    membership_data.type = this->entity.type;

    return membership_data;
}

MembershipEntity MembershipDAO::entity_from_membership_data(const Pubnub::ChatMembershipData& membership_data){
    MembershipEntity entity;
    entity.custom_field = membership_data.custom_data_json;
    entity.status = membership_data.status;
    entity.type = membership_data.type;

    return entity;
}
