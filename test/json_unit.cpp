#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(JsonTests);
BeforeEach(JsonTests) {/* no setup needed */}
AfterEach(JsonTests) {/* no cleanup needed */}

Ensure(JsonTests, json_unit) {
    assert_true(true);
}


