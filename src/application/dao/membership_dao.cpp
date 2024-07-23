#include "membership_dao.hpp"
#include "domain/membership_entity.hpp"

MembershipDAO::MembershipDAO(const Pubnub::String& custom_data) :
    entity({custom_data})
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

Pubnub::String MembershipDAO::to_custom_data() const {
    return this->entity.custom_field;
}

