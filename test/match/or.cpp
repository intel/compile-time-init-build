#include "test_matcher.hpp"

#include <match/ops.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("OR fulfils matcher concept", "[match or]") {
    using T = match::or_t<test_matcher, test_matcher>;
    static_assert(match::matcher<T>);
    static_assert(match::matcher_for<T, int>);
}

TEST_CASE("OR describes itself", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    static_assert(e.describe() == format("({}) or ({})"_sc,
                                         test_m<0>{}.describe(),
                                         test_m<1>{}.describe()));
}

TEST_CASE("OR description flattens", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{} or test_m<2>{};
    static_assert(e.describe() ==
                  format("({}) or ({}) or ({})"_sc, test_m<0>{}.describe(),
                         test_m<1>{}.describe(), test_m<2>{}.describe()));
}

TEST_CASE("OR describes a match", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    static_assert(e.describe_match(1) == format("({}) or ({})"_sc,
                                                test_m<0>{}.describe_match(1),
                                                test_m<1>{}.describe_match(1)));
}

TEST_CASE("OR match description flattens", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{} or test_m<2>{};
    static_assert(e.describe_match(1) == format("({}) or ({}) or ({})"_sc,
                                                test_m<0>{}.describe_match(1),
                                                test_m<1>{}.describe_match(1),
                                                test_m<2>{}.describe_match(1)));
}

TEST_CASE("OR matches correctly", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    static_assert(
        std::is_same_v<decltype(e), match::or_t<test_m<0>, test_m<1>> const>);
    static_assert(e(1));
    static_assert(not e(0));
}

TEST_CASE("OR simplifies correctly", "[match or]") {
    constexpr auto e = test_matcher{} and test_matcher{};
    static_assert(std::is_same_v<decltype(e), test_matcher const>);
}

TEST_CASE("any expression simplifies", "[match or]") {
    constexpr auto m = match::any(test_m<0>{}, test_m<1>{}, match::never);
    static_assert(
        std::is_same_v<decltype(m), match::or_t<test_m<0>, test_m<1>> const>);
}
