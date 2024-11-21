#include <lookup/input.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <type_traits>

TEST_CASE("an input with no entries (type deduced)", "[input]") {
    constexpr auto input = lookup::input{1};
    CHECK(input.default_value == 1);
    static_assert(
        std::is_same_v<decltype(input), lookup::input<int, int, 0> const>);
    CHECK(std::size(input) == 0);
}

TEST_CASE("an input with no entries (explicit types)", "[input]") {
    constexpr auto input = lookup::input<float, int>{1};
    CHECK(input.default_value == 1);
    static_assert(
        std::is_same_v<decltype(input), lookup::input<float, int, 0> const>);
    CHECK(std::size(input) == 0);
}

TEST_CASE("an input with some entries (type deduced)", "[input]") {
    constexpr auto input = lookup::input(1, std::array{lookup::entry{1.0f, 2}});
    CHECK(input.default_value == 1);
    static_assert(
        std::is_same_v<decltype(input), lookup::input<float, int, 1> const>);
    CHECK(std::size(input) == 1);
}
