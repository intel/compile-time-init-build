#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("X => X", "[match implies]") {
    STATIC_REQUIRE(match::implies(test_matcher{}, test_matcher{}));
}

TEST_CASE("false => X", "[match implies]") {
    STATIC_REQUIRE(match::implies(match::never, test_matcher{}));
}

TEST_CASE("X => true", "[match implies]") {
    STATIC_REQUIRE(match::implies(test_matcher{}, match::always));
}

TEST_CASE("disambiguate: false => true", "[match implies]") {
    STATIC_REQUIRE(match::implies(match::never, match::always));
}

TEST_CASE("X => (X ∨ Y)", "[match implies]") {
    STATIC_REQUIRE(
        match::implies(test_matcher{}, match::or_t<test_matcher, test_m<0>>{}));
}

TEST_CASE("recursive: X => (Y ∨ (X ∨ Z))", "[match implies]") {
    STATIC_REQUIRE(match::implies(
        test_matcher{},
        match::or_t<test_m<0>, match::or_t<test_matcher, test_m<1>>>{}));
}

TEST_CASE("(X ∧ Y) => X", "[match implies]") {
    STATIC_REQUIRE(match::implies(match::and_t<test_matcher, test_m<0>>{},
                                  test_matcher{}));
}

TEST_CASE("recursive: (Y ∧ (X ∧ Z)) => X", "[match implies]") {
    STATIC_REQUIRE(match::implies(
        match::and_t<test_m<0>, match::and_t<test_matcher, test_m<1>>>{},
        test_matcher{}));
}

TEST_CASE("ambiguity between X => or, and => X", "[match implies]") {
    using T1 = match::and_t<test_m<0>, test_m<1>>;
    using T2 = match::or_t<test_m<0>, test_m<1>>;
    STATIC_REQUIRE(match::implies(T1{}, T2{}));
}
