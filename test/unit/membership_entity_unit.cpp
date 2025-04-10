#include "domain/membership_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(MembershipEntities);
BeforeEach(MembershipEntities) {/* no setup needed */}
AfterEach(MembershipEntities) {/* no cleanup needed */}

Ensure(MembershipEntities, should_be_parsed_from_json) {
    auto result = MembershipEntity::from_json(Json::parse("{\"custom\":\"data\",\"status\":\"status\",\"type\":\"type\"}"));

    assert_string_equal(result.custom_field.c_str(), "\"data\"");
    assert_string_equal(result.status.c_str(), "status");
    assert_string_equal(result.type.c_str(), "type");
}

Ensure(MembershipEntities, should_update) {
    MembershipEntity base {"base", "base_status", "base_type"};
    MembershipEntity update {"update", "update_status", "update_type"};
    MembershipEntity empty_update {""};

    auto updated = MembershipEntity::from_base_and_updated_membership(base, update);
    auto empty_updated = MembershipEntity::from_base_and_updated_membership(base, empty_update);

    assert_string_equal(updated.custom_field.c_str(), "update");
    assert_string_equal(empty_updated.custom_field.c_str(), "base");
    assert_string_equal(updated.status.c_str(), "update_status");
    assert_string_equal(empty_updated.status.c_str(), "base_status");
    assert_string_equal(updated.type.c_str(), "update_type");
    assert_string_equal(empty_updated.type.c_str(), "base_type");
}
