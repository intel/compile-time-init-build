#include <sc/string_constant.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("creation", "[sc::string_constant]") {
    [[maybe_unused]] auto hi = "Hello, cruel world!"_sc;
    [[maybe_unused]] auto empty = ""_sc;
}

TEST_CASE("substr", "[sc::string_constant]") {
    auto hi = "Hello, cruel world!"_sc;
    auto sub = hi.substr(sc::int_<0>, sc::int_<5>);
    static_assert(sub == "Hello"_sc);
}

TEST_CASE("equality", "[sc::string_constant]") {
    static_assert("hi"_sc == "hi"_sc);
    static_assert(!("hi"_sc == "hello"_sc));

    static_assert("hi"_sc != "hello"_sc);
    static_assert(!("hello"_sc != "hello"_sc));
}

TEST_CASE("comparison", "[sc::string_constant]") {
    static_assert("abc"_sc < "abd"_sc);
    static_assert("abcd"_sc > "abc"_sc);
    static_assert("abc"_sc <= "abd"_sc);
    static_assert("abcd"_sc >= "abc"_sc);
}

TEST_CASE("join", "[sc::string_constant]") {
    static_assert("Hello, "_sc + "cruel world!"_sc == "Hello, cruel world!"_sc);
    static_assert(""_sc + "Luke"_sc == "Luke"_sc);
    static_assert("Computer"_sc + ""_sc == "Computer"_sc);
}
