#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <stdx/ct_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("AND fulfils matcher concept", "[match and]") {
    using T = match::and_t<test_matcher, test_matcher>;
    STATIC_REQUIRE(match::matcher<T>);
    STATIC_REQUIRE(match::matcher_for<T, int>);
}

TEST_CASE("AND describes itself", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    STATIC_REQUIRE(e.describe() ==
                   stdx::ct_format<"({}) and ({})">(test_m<0>{}.describe(),
                                                    test_m<1>{}.describe()));
}

TEST_CASE("AND description flattens", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{} and test_m<2>{};
    STATIC_REQUIRE(e.describe() == stdx::ct_format<"({}) and ({}) and ({})">(
                                       test_m<0>{}.describe(),
                                       test_m<1>{}.describe(),
                                       test_m<2>{}.describe()));
}

TEST_CASE("AND describes a match", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    STATIC_REQUIRE(e.describe_match(1) == stdx::ct_format<"({}) and ({})">(
                                              test_m<0>{}.describe_match(1),
                                              test_m<1>{}.describe_match(1)));
}

TEST_CASE("AND match description flattens", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{} and test_m<2>{};
    STATIC_REQUIRE(e.describe_match(1) ==
                   stdx::ct_format<"({}) and ({}) and ({})">(
                       test_m<0>{}.describe_match(1),
                       test_m<1>{}.describe_match(1),
                       test_m<2>{}.describe_match(1)));
}

TEST_CASE("AND matches correctly", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(e), match::and_t<test_m<0>, test_m<1>> const>);
    STATIC_REQUIRE(e(1));
    STATIC_REQUIRE(not e(0));
}

TEST_CASE("AND simplifies correctly", "[match and]") {
    constexpr auto e = test_matcher{} and test_matcher{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), test_matcher const>);
}

TEST_CASE("all expression simplifies", "[match and]") {
    constexpr auto m = match::all(test_m<0>{}, test_m<1>{}, match::always);
    STATIC_REQUIRE(
        std::is_same_v<decltype(m), match::and_t<test_m<0>, test_m<1>> const>);
}
