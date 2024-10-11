#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "string.hpp"

using namespace cgreen;

Describe(StringTests);
BeforeEach(StringTests) {/* no setup needed */}
AfterEach(StringTests) {/* no cleanup needed */}

Ensure(StringTests, keeps_proper_value) {
    Pubnub::String ctor("test");
    Pubnub::String assign = "test";

    assert_string_equal(ctor.c_str(), "test");
    assert_string_equal(assign.c_str(), "test");
}

Ensure(StringTests, concatenate) {
    Pubnub::String str1("test");
    Pubnub::String str2("concatenate");
    Pubnub::String str3("test");

    Pubnub::String result = str1 + str2;
    str3 += str2;

    assert_string_equal(result.c_str(), "testconcatenate");
    assert_string_equal(str3.c_str(), "testconcatenate");
}

Ensure(StringTests, compare) {
    Pubnub::String str1("test");
    Pubnub::String str2("test");

    assert_true(str1 == str2);
    assert_false(str1 != str2);
    assert_string_equal(str1.c_str(), str2.c_str());
}

Ensure(StringTests, std_string) {
    Pubnub::String sut("test");
    std::string std= sut.to_std_string();

    assert_string_equal(std.c_str(), "test");
}

Ensure(StringTests, lenght) {
    Pubnub::String sut("test");

    assert_equal(sut.length(), 4);
}

Ensure(StringTests, capacity) {
    Pubnub::String sut("test");

    assert_equal(sut.capacity(), 4);
}

Ensure(StringTests, clear) {
    Pubnub::String sut("test");

    sut.clear();

    assert_string_equal(sut.c_str(), "");
    assert_equal(sut.length(), 0);
    assert_equal(sut.capacity(), 4);
}

Ensure(StringTests, erase) {
    Pubnub::String sut("test");

    sut.erase(0, 1);

    assert_string_equal(sut.c_str(), "est");
    assert_equal(sut.length(), 3);
    assert_equal(sut.capacity(), 4);
}

Ensure(StringTests, insert) {
    Pubnub::String sut("test");

    sut.insert(1, 'i');

    assert_string_equal(sut.c_str(), "tiest");
}

Ensure(StringTests, reserve) {
    Pubnub::String sut;

    sut.reserve(10);

    assert_equal(sut.capacity(), 10);
}

Ensure(StringTests, shrink) {
    Pubnub::String sut;

    sut.reserve(10);

    sut += "test";

    sut.shrink();

    assert_equal(sut.capacity(), 4);
}

Ensure(StringTests, find) {
    Pubnub::String sut("test");

    assert_equal(sut.find("es"), 1);
    assert_equal(sut.find("shouldn't find me"), Pubnub::String::npos);
}

Ensure(StringTests, replace) {
    Pubnub::String sut("Hello, world!");

    sut.replace(7, 5, "Universe");

    assert_string_equal(sut.c_str(), "Hello, Universe!");
}

Ensure(StringTests, substring) {
    Pubnub::String sut("Hello, world!");

    Pubnub::String world = sut.substring(7, 5);
    Pubnub::String empty = sut.substring(100, 0);


    assert_string_equal(world.c_str(), "world");
    assert_string_equal(empty.c_str(), nullptr);
}

Ensure(StringTests, iterators) {
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



