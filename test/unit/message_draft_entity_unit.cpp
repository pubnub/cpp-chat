#include "domain/message_draft_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(MessageDraftEntities);
BeforeEach(MessageDraftEntities) {/* no setup needed */}
AfterEach(MessageDraftEntities) {/* no cleanup needed */}

Ensure(MessageDraftEntities, should_insert_plain_text) {
    MessageDraftEntity sut;

    sut.insert_text(0, "text><text");
    sut.insert_text(5, "text");

    assert_string_equal(sut.value.c_str(), "text>text<text");
}

Ensure(MessageDraftEntities, should_append_plain_text) {
    MessageDraftEntity sut;

    sut.insert_text(0, "Hello");
    sut.insert_text(5, " World");

    assert_string_equal(sut.value.c_str(), "Hello World");
}
