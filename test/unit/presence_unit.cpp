#include "domain/presence.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(Presences);
BeforeEach(Presences) {/* no setup needed */}
AfterEach(Presences) {/* no cleanup needed */}

Ensure(Presences, should_parse_users_from_payload) {
    auto payload = Json::parse("{\"uuids\": [\"user1\", \"user2\"]}");

    auto users = Presence::users_from_response(payload);

    assert_that(users.size(), is_equal_to(2));
    assert_true(vector_contains(users, Pubnub::String("user1")));
    assert_true(vector_contains(users, Pubnub::String("user2")));
}

Ensure(Presences, should_parse_channels_from_payload) {
    auto payload = Json::parse("{\"payload\": {\"channels\": [\"channel1\", \"channel2\"]}}");

    auto channels = Presence::channels_from_response(payload);

    assert_that(channels.size(), is_equal_to(2));
    assert_true(vector_contains(channels, Pubnub::String("channel1")));
    assert_true(vector_contains(channels, Pubnub::String("channel2")));
}
