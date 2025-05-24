#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace match;

TEST_CASE("Negate true: ¬T -> F", "[match simplify not]") {
    constexpr auto e = not_t<always_t>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), never_t const>);
}

TEST_CASE("Negate false: ¬F -> T", "[match simplify not]") {
    constexpr auto e = not_t<never_t>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), always_t const>);
}

TEST_CASE("Double negation: ¬¬X -> X", "[match simplify not]") {
    constexpr auto e = not_t<not_t<test_matcher>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_matcher const>);
}
