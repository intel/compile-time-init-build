#include <sc/format.hpp>
#include <sc/fwd.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <type_traits>

TEST_CASE("format a static string", "[sc::format]") {
    static_assert(sc::format("Hello"_sc) == "Hello"_sc);
}

TEST_CASE("interpolate 1 string", "[sc::format]") {
    static_assert(sc::format("Hello, {} is a good day!"_sc, "today"_sc) ==
                  "Hello, today is a good day!"_sc);
}

TEST_CASE("ignore escaped curly braces", "[sc::format]") {
    static_assert(sc::format("Hello, {} {{is}} a good day!"_sc, "today"_sc) ==
                  "Hello, today {{is}} a good day!"_sc);
}

TEST_CASE("interpolate N strings", "[sc::format]") {
    static_assert(sc::format("This {} is a {}."_sc, "box"_sc, "package"_sc) ==
                  "This box is a package."_sc);
    static_assert(sc::format("{} {} {} arguments."_sc, "There"_sc, "are"_sc,
                             "three"_sc) == "There are three arguments."_sc);
}

TEST_CASE("interpolate empty string", "[sc::format]") {
    static_assert(sc::format("[{}]"_sc, ""_sc) == "[]"_sc);
}

namespace {
template <typename T, T Value>
struct my_integral_constant : public std::integral_constant<T, Value> {};
} // namespace

TEST_CASE("interpolate integral_constant", "[sc::format]") {
    static_assert(sc::format("The answer is {}."_sc, sc::int_<42>) ==
                  "The answer is 42."_sc);
    static_assert(format("{}"_sc, my_integral_constant<int, 42>{}) == "42"_sc);
}

TEST_CASE("interpolate mixed types", "[sc::format]") {
    static_assert(sc::format("Only {} more days until {}."_sc, sc::int_<100>,
                             "retirement"_sc) ==
                  "Only 100 more days until retirement."_sc);
}

namespace {
enum class cmd { READ, WRITE };
}

TEST_CASE("interpolate enum", "[sc::format]") {
    static_assert(sc::format("Command = {}"_sc, sc::enum_<cmd::WRITE>) ==
                  "Command = WRITE"_sc);
    static_assert(sc::format("Command = {}"_sc, sc::enum_<cmd::READ>) ==
                  "Command = READ"_sc);
}

struct complete {};
struct incomplete;

TEST_CASE("interpolate typenames", "[sc::format]") {
    static_assert(sc::format("Type = {}"_sc, sc::type_<complete>) ==
                  "Type = complete"_sc);
    static_assert(sc::format("Type = {}"_sc, sc::type_<incomplete>) ==
                  "Type = incomplete"_sc);
}

TEST_CASE("int formatting options", "[sc::format]") {
    static_assert(sc::format("{:d}"_sc, sc::int_<42>) == "42"_sc);
    static_assert(sc::format("{:b}"_sc, sc::int_<17>) == "10001"_sc);
    static_assert(sc::format("{:x}"_sc, sc::int_<0xba115>) == "ba115"_sc);
    static_assert(sc::format("{:X}"_sc, sc::int_<0xba115>) == "BA115"_sc);
    static_assert(sc::format("{:o}"_sc, sc::int_<16>) == "20"_sc);
    static_assert(sc::format("{:08x}"_sc, sc::int_<0xbea75>) == "000bea75"_sc);
    static_assert(sc::format("{:8x}"_sc, sc::int_<0xbea75>) == "   bea75"_sc);
    static_assert(sc::format("{:4x}"_sc, sc::int_<0xbea75>) == "bea75"_sc);
    static_assert(sc::format("{:04x}"_sc, sc::int_<0xbea75>) == "bea75"_sc);
}

TEST_CASE("runtime integral values", "[sc::format]") {
    static_assert(sc::format("{}"_sc, 0) ==
                  (sc::lazy_string_format{"{}"_sc, cib::make_tuple(0)}));
    static_assert(sc::format("{}"_sc, 1) ==
                  (sc::lazy_string_format{"{}"_sc, cib::make_tuple(1)}));
    static_assert(sc::format("I am {} and my sister is {}"_sc, 6, 8) ==
                  (sc::lazy_string_format{"I am {} and my sister is {}"_sc,
                                          cib::make_tuple(6, 8)}));
    static_assert(sc::format("{}"_sc, 100) !=
                  (sc::lazy_string_format{"{}"_sc, cib::make_tuple(99)}));
    static_assert(sc::format("{}"_sc, true) ==
                  (sc::lazy_string_format{"{}"_sc, cib::make_tuple(true)}));
}

TEST_CASE("mixed runtime and compile time values", "[sc::format]") {
    static_assert(
        sc::format("ct value {} and rt value {} mixed"_sc, "ctval"_sc, 1) ==
        (sc::lazy_string_format{"ct value ctval and rt value {} mixed"_sc,
                                cib::make_tuple(1)}));
    static_assert(
        sc::format("rt value {} and ct value {} mixed"_sc, 1, "ctval"_sc) ==
        (sc::lazy_string_format{"rt value {} and ct value ctval mixed"_sc,
                                cib::make_tuple(1)}));
}

TEST_CASE("format a formatted string", "[sc::format]") {
    static_assert(sc::format("Hello {}!"_sc, sc::format("World"_sc)) ==
                  "Hello World!"_sc);

    static_assert(
        sc::format("The value is {}."_sc, sc::format("(year={})"_sc, 2022)) ==
        (sc::lazy_string_format{"The value is (year={})."_sc,
                                cib::make_tuple(2022)}));

    static_assert(sc::format("a{}b{}c"_sc, sc::format("1{}2{}3"_sc, 10, 20),
                             sc::format("4{}5{}6"_sc, 30, 40)) ==
                  (sc::lazy_string_format{"a1{}2{}3b4{}5{}6c"_sc,
                                          cib::make_tuple(10, 20, 30, 40)}));
}
