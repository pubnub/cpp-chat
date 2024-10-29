#include "domain/message_draft_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(MessageDraftEntities);
BeforeEach(MessageDraftEntities) {/* no setup needed */}
AfterEach(MessageDraftEntities) {/* no cleanup needed */}

Ensure(MessageDraftEntities, should_insert_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "text><text")
        .insert_text(5, "text");

    assert_string_equal(result.value.c_str(), "text>text<text");
}

Ensure(MessageDraftEntities, should_append_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "Hello")
        .insert_text(5, " World");

    assert_string_equal(result.value.c_str(), "Hello World");
}

Ensure(MessageDraftEntities, should_remove_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "Hello World")
        .remove_text(5, 6);

    assert_string_equal(result.value.c_str(), "Hello");
}
