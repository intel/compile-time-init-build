#include <cib/core.hpp>

#include <catch2/catch_test_macros.hpp>

auto const sum = [](int elem, int state){
    return elem + state;
};

TEST_CASE("given an empty tuple", "[meta]") {
    auto const t = std::make_tuple();

    SECTION("the initial state is returned") {
        REQUIRE(cib::detail::fold_right(t, 9, sum) == 9);
    }
}

TEST_CASE("given a tuple of a single element", "[meta]") {
    auto const t = std::make_tuple(42);

    SECTION("it can be folded") {
        REQUIRE(cib::detail::fold_right(t, 0, sum) == 42);
    }

    SECTION("it can be folded with an initial state") {
        REQUIRE(cib::detail::fold_right(t, 10, sum) == 52);
    }
}

TEST_CASE("given a tuple of elements", "[meta]") {
    auto const t = std::make_tuple(1, 2, 3, 4, 5);

    SECTION("they can be folded") {
        REQUIRE(cib::detail::fold_right(t, 0, sum) == 15);
    }

    SECTION("they can be folded with initial state") {
        REQUIRE(cib::detail::fold_right(t, 5, sum) == 20);
    }
}