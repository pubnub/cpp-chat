#include "domain/json.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(Jsons);
BeforeEach(Jsons) {/* no setup needed */}
AfterEach(Jsons) {/* no cleanup needed */}

Ensure(Jsons, should_index_by_key) {
    auto sut = Json::parse("{\"key\": \"value\"}");

    assert_string_equal(sut["key"].dump(), "\"value\"");
}

Ensure(Jsons, should_index_by_integer) {
    auto sut = Json::parse("[\"value\"]");

    assert_string_equal(sut[0].dump(), "\"value\"");
}

Ensure(Jsons, should_contain) {
    auto sut = Json::parse("{\"key\": \"value\"}");

    assert_true(sut.contains("key"));
}

