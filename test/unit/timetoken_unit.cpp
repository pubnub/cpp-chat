#include "domain/timetoken.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/legacy.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(Timetokens);
BeforeEach(Timetokens) {/* no setup needed */}
AfterEach(Timetokens) {/* no cleanup needed */}

Ensure(Timetokens, should_return_current_timetoken) {
    // TODO: mock time
}

Ensure(Timetokens, should_increase_timetoken_by_value) {
    assert_string_equal(Timetoken::increase_by("12345678901234567", 1).c_str(), "12345678901234568");
    assert_string_equal(Timetoken::increase_by("12345678901234567", 10).c_str(), "12345678901234577");
} 

Ensure(Timetokens, should_increase_timetoken_by_one) {
    assert_string_equal(Timetoken::increase_by_one("12345678901234567").c_str(), "12345678901234568");
}

Ensure(Timetokens, should_convert_timetoken_to_long) {
    assert_that(Timetoken::to_long("12345678901234567"), is_equal_to(12345678901234567));
}

