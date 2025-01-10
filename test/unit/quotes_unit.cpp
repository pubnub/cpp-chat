#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "domain/quotes.hpp"

using namespace cgreen;

Describe(Quote);
BeforeEach(Quote) {/* no setup needed */}
AfterEach(Quote) {/* no cleanup needed */}

Ensure(Quote, should_add_quotes_to_string) {
    assert_string_equal(Quotes::add("message").c_str(), "\"message\"");
}

Ensure(Quote, should_not_add_quotes_to_quoted_string) {
    assert_string_equal(Quotes::add("\"message\"").c_str(), "\"message\"");
}

Ensure(Quote, should_remove_quotes_from_quoted_string) {
    assert_string_equal(Quotes::remove("\"message\"").c_str(), "message");
}

Ensure(Quote, should_not_remove_quotes_from_string_where_there_are_no_quotes) {
    assert_string_equal(Quotes::remove("message").c_str(), "message");
}

Ensure(Quote, should_return_true_for_quoted_string) {
    assert_that(Quotes::is_quoted("\"message\""), is_true);
}

Ensure(Quote, should_return_false_for_string) {
    assert_that(Quotes::is_quoted("message"), is_false);
}
