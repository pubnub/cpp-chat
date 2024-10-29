#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "string.hpp"

using namespace cgreen;

Describe(Strings);
BeforeEach(Strings) {/* no setup needed */}
AfterEach(Strings) {/* no cleanup needed */}

Ensure(Strings, should_keeps_proper_value) {
    Pubnub::String ctor("test");
    Pubnub::String assign = "test";

    assert_string_equal(ctor.c_str(), "test");
    assert_string_equal(assign.c_str(), "test");
}

Ensure(Strings, should_concatenate) {
    Pubnub::String str1("test");
    Pubnub::String str2("concatenate");
    Pubnub::String str3("test");

    Pubnub::String result = str1 + str2;
    str3 += str2;

    assert_string_equal(result.c_str(), "testconcatenate");
    assert_string_equal(str3.c_str(), "testconcatenate");
}

Ensure(Strings, should_compare) {
    Pubnub::String str1("test");
    Pubnub::String str2("test");

    assert_true(str1 == str2);
    assert_false(str1 != str2);
    assert_string_equal(str1.c_str(), str2.c_str());
}

Ensure(Strings, should_work_with_std_string) {
    Pubnub::String sut("test");
    std::string std= sut.to_std_string();

    assert_string_equal(std.c_str(), "test");
}

Ensure(Strings, should_know_its_lenght) {
    Pubnub::String sut("test");

    assert_equal(sut.length(), 4);
}

Ensure(Strings, should_know_its_capacity) {
    Pubnub::String sut("test");

    assert_equal(sut.capacity(), 4);
}

Ensure(Strings, shouls_clear_itself) {
    Pubnub::String sut("test");

    sut.clear();

    assert_string_equal(sut.c_str(), "");
    assert_equal(sut.length(), 0);
    assert_equal(sut.capacity(), 4);
}

Ensure(Strings, should_erase) {
    Pubnub::String sut("test");

    sut.erase(0, 1);

    assert_string_equal(sut.c_str(), "est");
    assert_equal(sut.length(), 3);
    assert_equal(sut.capacity(), 4);
}

Ensure(Strings, should_insert) {
    Pubnub::String sut("test");

    sut.insert(1, 'i');

    assert_string_equal(sut.c_str(), "tiest");
}

Ensure(Strings, should_reserve) {
    Pubnub::String sut;

    sut.reserve(10);

    assert_equal(sut.capacity(), 10);
}

Ensure(Strings, should_shrink) {
    Pubnub::String sut;

    sut.reserve(10);

    sut += "test";

    sut.shrink();

    assert_equal(sut.capacity(), 4);
}

Ensure(Strings, should_find) {
    Pubnub::String sut("test");

    assert_equal(sut.find("es"), 1);
    assert_equal(sut.find("shouldn't find me"), Pubnub::String::npos);
}

Ensure(Strings, should_replace) {
    Pubnub::String sut("Hello, world!");

    sut.replace(7, 5, "Universe");

    assert_string_equal(sut.c_str(), "Hello, Universe!");
}

Ensure(Strings, should_replace_end_of_string) {
    Pubnub::String sut("Hello, world!");

    sut.replace(7, 6, "Universe?");

    assert_string_equal(sut.c_str(), "Hello, Universe?");
}

Ensure(Strings, should_replace_append_data_with_zero_count) {
    Pubnub::String sut("Hello,");

    sut.replace(6, 0, " world!");

    assert_string_equal(sut.c_str(), "Hello, world!");
}

Ensure(Strings, should_replace_insert_data_with_zero_count) {
    Pubnub::String sut("Hello,!");

    sut.replace(6, 0, " world");

    assert_string_equal(sut.c_str(), "Hello, world!");
}

Ensure(Strings, should_replace_properly_when_data_is_huge) {
    Pubnub::String sut("1234><5678");

    sut.replace(5, 0, "abc");

    assert_string_equal(sut.c_str(), "1234>abc<5678");
}

Ensure(Strings, should_substring) {
    Pubnub::String sut("Hello, world!");

    Pubnub::String world = sut.substring(7, 5);
    Pubnub::String empty = sut.substring(100, 0);


    assert_string_equal(world.c_str(), "world");
    assert_string_equal(empty.c_str(), nullptr);
}

Ensure(Strings, should_work_with_iterators) {
    Pubnub::String sut("Hello, world!");

    int count_of_chars = 0;

    // immutable iterator
    for (auto it = sut.begin(); it != sut.end(); it++) {
        count_of_chars++;
    }

    assert_equal(count_of_chars, 13);

    // mutable iterator
    for (auto it = sut.begin(); it != sut.end(); it++) {
        *it = 'a';
    }

    assert_string_equal(sut.c_str(), "aaaaaaaaaaaaa");
}



