#include "test_matcher.hpp"

#include <match/ops.hpp>
#include <match/sum_of_products.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace match;

TEST_CASE("Single term: X -> X", "[match sum of products]") {
    constexpr auto e = test_matcher{};
    constexpr auto s = sum_of_products(e);
    static_assert(std::is_same_v<decltype(s), test_matcher const>);
}

TEST_CASE("Negation: ¬X -> ¬X", "[match sum of products]") {
    constexpr auto e = not_t<test_matcher>{};
    constexpr auto s = sum_of_products(e);
    static_assert(std::is_same_v<decltype(s), not_t<test_matcher> const>);
}

TEST_CASE("Disjunction: X ∨ Y -> X ∨ Y", "[match sum of products]") {
    constexpr auto e = or_t<test_m<0>, test_m<1>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s), or_t<test_m<0>, test_m<1>> const>);
}

TEST_CASE("Left distributive law: X ∧ (Y ∨ Z) -> (X ∧ Y) ∨ (X ∧ Z)",
          "[match sum of products]") {
    constexpr auto e = and_t<test_m<0>, or_t<test_m<1>, test_m<2>>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s), or_t<and_t<test_m<0>, test_m<1>>,
                                         and_t<test_m<0>, test_m<2>>> const>);
}

TEST_CASE("Right distributive law: (X ∨ Y) ∧ Z -> (X ∧ Z) ∨ (Y ∧ Z)",
          "[match sum of products]") {
    constexpr auto e = and_t<or_t<test_m<0>, test_m<1>>, test_m<2>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s), or_t<and_t<test_m<0>, test_m<2>>,
                                         and_t<test_m<1>, test_m<2>>> const>);
}

TEST_CASE("Binary distributive law: (A ∨ B) ∧ (X ∨ Y) -> (A ∧ X) ∨ (A ∧ Y) ∨ "
          "(B ∧ X) ∨ (B ∧ Y)",
          "[match sum of products]") {
    constexpr auto e =
        and_t<or_t<test_m<0>, test_m<1>>, or_t<test_m<2>, test_m<3>>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<
            decltype(s),
            or_t<or_t<and_t<test_m<0>, test_m<2>>, and_t<test_m<0>, test_m<3>>>,
                 or_t<and_t<test_m<1>, test_m<2>>,
                      and_t<test_m<1>, test_m<3>>>> const>);
}

TEST_CASE(
    "Recursive inside disjunction: A ∨ (X ∧ (Y ∨ Z)) -> A ∨ (X ∧ Y) ∨ (X ∧ Z)",
    "[match sum of products]") {
    constexpr auto e =
        or_t<test_m<0>, and_t<test_m<1>, or_t<test_m<2>, test_m<3>>>>{};
    constexpr auto s = sum_of_products(simplify(e));
    static_assert(std::is_same_v<
                  decltype(s),
                  or_t<test_m<0>, or_t<and_t<test_m<1>, test_m<2>>,
                                       and_t<test_m<1>, test_m<3>>>> const>);
}

TEST_CASE("Push negation inside conjunction: ¬(X ∧ Y) -> ¬X ∨ ¬Y",
          "[match sum of products]") {
    constexpr auto e = not_t<and_t<test_m<0>, test_m<1>>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s),
                       or_t<not_t<test_m<0>>, not_t<test_m<1>>> const>);
}

TEST_CASE("Push negation inside disjunction: ¬(X ∨ Y) -> ¬X ∧ ¬Y",
          "[match sum of products]") {
    constexpr auto e = not_t<or_t<test_m<0>, test_m<1>>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s),
                       and_t<not_t<test_m<0>>, not_t<test_m<1>>> const>);
}

TEST_CASE("Recursive inside NOT: ¬(X ∧ (Y ∨ Z)) -> ¬X ∨ (¬Y ∧ ¬Z)",
          "[match sum of products]") {
    constexpr auto e = not_t<and_t<test_m<0>, or_t<test_m<1>, test_m<2>>>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<decltype(s),
                       or_t<not_t<test_m<0>>,
                            and_t<not_t<test_m<1>>, not_t<test_m<2>>>> const>);
}

TEST_CASE("Recursive inside AND: ¬(X ∨ Y) ∧ Z -> ¬X ∧ ¬Y ∧ ¬Z",
          "[match sum of products]") {
    constexpr auto e = and_t<not_t<or_t<test_m<0>, test_m<1>>>, test_m<2>>{};
    constexpr auto s = sum_of_products(e);
    static_assert(
        std::is_same_v<
            decltype(s),
            and_t<and_t<not_t<test_m<0>>, not_t<test_m<1>>>, test_m<2>> const>);
}
