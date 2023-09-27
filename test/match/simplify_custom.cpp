#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <type_traits>

using namespace match;

TEST_CASE("custom matcher simplifies (NOT)", "[match simplify]") {
    constexpr auto e = not rel_matcher<std::less<>, 5>{};
    static_assert(std::is_same_v<decltype(e),
                                 rel_matcher<std::greater_equal<>, 5> const>);
}

TEST_CASE("custom matcher simplifies (AND)", "[match simplify]") {
    constexpr auto e = rel_matcher<std::less<>, 5>{} and
                       rel_matcher<std::greater_equal<>, 5>{};
    static_assert(std::is_same_v<decltype(e), never_t const>);
}

TEST_CASE("custom matcher simplifies (OR)", "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} or rel_matcher<std::greater_equal<>, 5>{};
    static_assert(std::is_same_v<decltype(e), always_t const>);
}
