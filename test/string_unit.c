#include "cgreen/assertions.h"
#include "cgreen/cgreen.h"

Describe(StringTests);

Ensure(StringTests, can_compare_strings) {
    char *string1 = "Hello, world!";
    char *string2 = "Hello, world!";
    assert_string_equal(string1, string2);
}

Ensure(StringTests, can_compare_strings_with_different_lengths) {
    char *string1 = "Hello, world!";
    char *string2 = "Hello, world";
    assert_string_not_equal(string1, string2);
}

Ensure(StringTests, can_compare_strings_with_different_content) {
    char *string1 = "Hello, world!";
    char *string2 = "Hello, world?";
    assert_string_not_equal(string1, string2);
}


