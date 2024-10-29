#include "domain/restrictions.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(Restriction);
BeforeEach(Restriction) {/* no setup needed */}
AfterEach(Restriction) {/* no cleanup needed */}

Ensure(Restriction, should_prepare_remove_member_payload) {
    auto result = trim_whitespaces(Restrictions::remove_member_payload("user1"));
    assert_string_equal(result.c_str(), "[{\"uuid\":{\"id\":\"user1\"}}]");
}

Ensure(Restriction, should_prepare_lift_restrictions_payload) {
    auto result = trim_whitespaces(Restrictions::lift_restrictions_payload("channel1", "reason"));
    assert_string_equal(result.c_str(), "{\"channelId\":\"PUBNUB_INTERNAL_MODERATION_channel1\",\"restrictions\":\"lifted\",\"reason\":\"reason\"}");
}

Ensure(Restriction, should_prepare_restrict_member_payload) {
    auto result = trim_whitespaces(Restrictions::restrict_member_payload("user1"));
    assert_string_equal(result.c_str(), "[{\"uuid\":{\"id\":\"user1\"}}]");
}

