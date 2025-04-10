#include "domain/typing.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(Typings);
BeforeEach(Typings) {/* no setup needed */}
AfterEach(Typings) {/* no cleanup needed */}

Ensure(Typings, should_start_typing) {
    Typing sut;

    assert_false(sut.is_sent());

    sut.start();

    assert_true(sut.is_sent());
}

Ensure(Typings, should_stop_typing) {
    Typing sut;

    sut.start();
    sut.stop();

    assert_false(sut.is_sent());
}

Ensure(Typings, should_handle_typing_users) {
    Typing sut;

    sut.add_typing_indicator("user1");

    assert_true(sut.contains_typing_indicator("user1"));
}

Ensure(Typings, should_remove_typing_users) {
    Typing sut;

    sut.add_typing_indicator("user1");
    sut.remove_typing_indicator("user1");
    sut.remove_typing_indicator("user2");

    assert_false(sut.contains_typing_indicator("user1"));
    assert_false(sut.contains_typing_indicator("user2"));
}

Ensure(Typings, should_return_typing_users) {
    Typing sut;

    sut.add_typing_indicator("user1");
    sut.add_typing_indicator("user2");

    for (auto user : sut.get_typing_indicators())
    {
        assert_true(user == Pubnub::String("user1") || user == Pubnub::String("user2"));
    }
}

Ensure(Typings, should_prepare_payload_for_typing_user) {
    auto result = trim_whitespaces(Typing::payload(true));

    assert_false(result.find("\"value\":true") == Pubnub::String::npos);
}

Ensure(Typings, should_prepare_payload_for_stopped_typing_user) {
    auto result = trim_whitespaces(Typing::payload(false));

    assert_false(result.find("\"value\":false") == Pubnub::String::npos);
}

Ensure(Typings, should_parse_event_for_typing_state) {
    Pubnub::Event event;
    event.payload = "{\"value\":true}";

    auto result = Typing::typing_value_from_event(event);

    assert_true(result.has_value());
    assert_true(result.value());
}


