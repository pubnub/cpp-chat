#include "domain/membership_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(MembershipEntities);
BeforeEach(MembershipEntities) {/* no setup needed */}
AfterEach(MembershipEntities) {/* no cleanup needed */}

Ensure(MembershipEntities, should_be_parsed_from_json) {
    auto result = MembershipEntity::from_json(Json::parse("{\"custom\":\"data\"}"));

    assert_string_equal(result.custom_field.c_str(), "\"data\"");
}

Ensure(MembershipEntities, should_update) {
    MembershipEntity base {"base"};
    MembershipEntity update {"update"};
    MembershipEntity empty_update {""};

    auto updated = MembershipEntity::from_base_and_updated_membership(base, update);
    auto empty_updated = MembershipEntity::from_base_and_updated_membership(base, empty_update);

    assert_string_equal(updated.custom_field.c_str(), "update");
    assert_string_equal(empty_updated.custom_field.c_str(), "base");
}
