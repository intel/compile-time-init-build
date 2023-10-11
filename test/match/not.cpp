#include "test_matcher.hpp"

#include <match/ops.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("NOT fulfils matcher concept", "[match not]") {
    using T = decltype(not test_matcher{});
    static_assert(match::matcher<T>);
    static_assert(match::matcher_for<T, int>);
}

TEST_CASE("NOT describes itself", "[match not]") {
    constexpr auto e = not test_matcher{};
    static_assert(e.describe() ==
                  format("not ({})"_sc, test_m<0>{}.describe()));
}

TEST_CASE("NOT describes a match", "[match not]") {
    constexpr auto e = not test_matcher{};
    static_assert(e.describe_match(1) ==
                  format("not ({})"_sc, test_matcher{}.describe_match(1)));
}

TEST_CASE("NOT matches correctly", "[match not]") {
    static_assert((not test_matcher{})(0));
    static_assert(not(not test_matcher{})(1));
}

TEST_CASE("NOT simplifies correctly", "[match not]") {
    constexpr auto e = not not test_matcher{};
    static_assert(std::is_same_v<decltype(e), test_matcher const>);
}
