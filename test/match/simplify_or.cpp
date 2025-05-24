#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace match;

TEST_CASE("Idempotence: X ∨ X -> X", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, test_matcher>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_matcher const>);
}

TEST_CASE("Right complementation: X ∨ ¬X -> T", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, not_t<test_matcher>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), always_t const>);
}

TEST_CASE("Left complementation: ¬X ∨ X -> F", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, not_t<test_matcher>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), always_t const>);
}

TEST_CASE("Right annihilation: X ∨ T -> T", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, always_t>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), always_t const>);
}

TEST_CASE("Left annihilation: T ∨ X -> T", "[match simplify or]") {
    constexpr auto e = or_t<always_t, test_matcher>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), always_t const>);
}

TEST_CASE("Right identity: X ∨ F -> X", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, never_t>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_matcher const>);
}

TEST_CASE("Left identity: F ∨ X -> X", "[match simplify or]") {
    constexpr auto e = or_t<never_t, test_matcher>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_matcher const>);
}

TEST_CASE("Left absorption: X ∨ (X ∧ Y) -> X", "[match simplify or]") {
    constexpr auto e = or_t<test_m<0>, and_t<test_m<0>, test_m<1>>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_m<0> const>);
}

TEST_CASE("Right absorption: (X ∧ Y) ∨ X -> X", "[match simplify or]") {
    constexpr auto e = or_t<and_t<test_m<0>, test_m<1>>, test_m<0>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_m<0> const>);
}

TEST_CASE("De Morgan's Law: ¬X ∨ ¬Y -> ¬(X ∧ Y)", "[match simplify or]") {
    constexpr auto e = or_t<not_t<test_m<0>>, not_t<test_m<1>>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(
        std::is_same_v<decltype(s), not_t<and_t<test_m<0>, test_m<1>>> const>);
}

TEST_CASE("OR simplifies recursively", "[match simplify or]") {
    constexpr auto e = or_t<test_matcher, or_t<test_matcher, never_t>>{};
    constexpr auto s = simplify(e);
    STATIC_REQUIRE(std::is_same_v<decltype(s), test_matcher const>);
}
