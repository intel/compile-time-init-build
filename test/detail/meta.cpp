#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

auto const sum = [](int elem, int state){
    return elem + state;
};

TEST_CASE("fold_right empty tuple", "[meta]") {
    auto const t = std::make_tuple();

    REQUIRE(cib::detail::fold_right(t, 9, sum) == 9);
}

TEST_CASE("fold_right single element tuple", "[meta]") {
    auto const t = std::make_tuple(42);

    REQUIRE(cib::detail::fold_right(t, 0, sum) == 42);
    REQUIRE(cib::detail::fold_right(t, 10, sum) == 52);
}

TEST_CASE("fold_right multi-element tuple", "[meta]") {
    auto const t = std::make_tuple(1, 2, 3, 4, 5);

    REQUIRE(cib::detail::fold_right(t, 0, sum) == 15);
    REQUIRE(cib::detail::fold_right(t, 5, sum) == 20);
}
