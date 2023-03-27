#include <sc/string_constant.hpp>
#include <sc/to_string_constant.hpp>

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

TEST_CASE("replace", "[sc::string_constant]") {
    auto str = "Hello George, my name is Luke."_sc;
    auto str2 = str.replace(sc::int_<6>, sc::int_<6>, "Frank"_sc);
    static_assert(str2 == "Hello Frank, my name is Luke."_sc);
}

TEST_CASE("conversion to int", "[sc::string_constant]") {
    static_assert(sc::to_int("0"_sc) == 0);

    static_assert(sc::to_int("1"_sc) == 1);
    static_assert(sc::to_int("10"_sc) == 10);
    static_assert(sc::to_int("2345"_sc) == 2345);
    static_assert(sc::to_int("2147483647"_sc) == 2147483647);

    static_assert(sc::to_int("-1"_sc) == -1);
    static_assert(sc::to_int("-732"_sc) == -732);
    static_assert(sc::to_int("-2147483647"_sc) == -2147483647);
    static_assert(sc::to_int("-2147483648"_sc) == -2147483648);
}

TEST_CASE("conversion from int", "[sc::string_constant]") {
    static_assert(sc::to_string_constant(sc::uint_<0>) == "0"_sc);
    static_assert(sc::to_string_constant(sc::int_<0>) == "0"_sc);

    static_assert(sc::to_string_constant(sc::int_<1>) == "1"_sc);
    static_assert(sc::to_string_constant(sc::int_<10>) == "10"_sc);
    static_assert(sc::to_string_constant(sc::int_<2345>) == "2345"_sc);
    static_assert(sc::to_string_constant(sc::int_<2147483647>) ==
                  "2147483647"_sc);

    static_assert(sc::to_string_constant(sc::int_<-1>) == "-1"_sc);
    static_assert(sc::to_string_constant(sc::int_<-732>) == "-732"_sc);
    static_assert(sc::to_string_constant(sc::int_<-2147483647>) ==
                  "-2147483647"_sc);
    static_assert(sc::to_string_constant(sc::int_<-2147483648>) ==
                  "-2147483648"_sc);

    static_assert(sc::to_string_constant(sc::int_<4>, sc::int_<2>) == "100"_sc);
    static_assert(sc::to_string_constant(sc::int_<10>, sc::int_<2>) ==
                  "1010"_sc);
    static_assert(sc::to_string_constant(sc::int_<8>, sc::int_<8>) == "10"_sc);
    static_assert(sc::to_string_constant(sc::int_<16>, sc::int_<8>) == "20"_sc);
    static_assert(sc::to_string_constant(sc::int_<16>, sc::int_<16>) ==
                  "10"_sc);
    static_assert(sc::to_string_constant(sc::int_<10>, sc::int_<16>) == "a"_sc);
    static_assert(sc::to_string_constant(sc::int_<0xca7b007>, sc::int_<16>) ==
                  "ca7b007"_sc);
    static_assert(sc::to_string_constant(sc::int_<0x101d0115>, sc::int_<16>) ==
                  "101d0115"_sc);
}
