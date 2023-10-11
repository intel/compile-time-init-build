#include "test_matcher.hpp"

#include <match/ops.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("AND fulfils matcher concept", "[match and]") {
    using T = match::and_t<test_matcher, test_matcher>;
    static_assert(match::matcher<T>);
    static_assert(match::matcher_for<T, int>);
}

TEST_CASE("AND describes itself", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    static_assert(e.describe() == format("({}) and ({})"_sc,
                                         test_m<0>{}.describe(),
                                         test_m<1>{}.describe()));
}

TEST_CASE("AND description flattens", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{} and test_m<2>{};
    static_assert(e.describe() ==
                  format("({}) and ({}) and ({})"_sc, test_m<0>{}.describe(),
                         test_m<1>{}.describe(), test_m<2>{}.describe()));
}

TEST_CASE("AND describes a match", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    static_assert(e.describe_match(1) == format("({}) and ({})"_sc,
                                                test_m<0>{}.describe_match(1),
                                                test_m<1>{}.describe_match(1)));
}

TEST_CASE("AND match description flattens", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{} and test_m<2>{};
    static_assert(e.describe_match(1) == format("({}) and ({}) and ({})"_sc,
                                                test_m<0>{}.describe_match(1),
                                                test_m<1>{}.describe_match(1),
                                                test_m<2>{}.describe_match(1)));
}

TEST_CASE("AND matches correctly", "[match and]") {
    constexpr auto e = test_m<0>{} and test_m<1>{};
    static_assert(
        std::is_same_v<decltype(e), match::and_t<test_m<0>, test_m<1>> const>);
    static_assert(e(1));
    static_assert(not e(0));
}

TEST_CASE("AND simplifies correctly", "[match and]") {
    constexpr auto e = test_matcher{} and test_matcher{};
    static_assert(std::is_same_v<decltype(e), test_matcher const>);
}

TEST_CASE("all expression simplifies", "[match and]") {
    constexpr auto m = match::all(test_m<0>{}, test_m<1>{}, match::always);
    static_assert(
        std::is_same_v<decltype(m), match::and_t<test_m<0>, test_m<1>> const>);
}
