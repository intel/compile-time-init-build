#include <lookup/input.hpp>
#include <lookup/linear_search_lookup.hpp>

#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using LS = lookup::linear_search_lookup<2>;
}

TEST_CASE("a lookup with more entries than allowed", "[linear search]") {
    constexpr auto lookup = LS::make(CX_VALUE(lookup::input<int, int, 3>{
        0, std::array{lookup::entry{1, 1}, lookup::entry{2, 2},
                      lookup::entry{3, 3}}}));
    STATIC_REQUIRE(lookup::strategy_failed(lookup));
}

TEST_CASE("a lookup with no entries", "[linear search]") {
    constexpr auto lookup = LS::make(CX_VALUE(lookup::input<int>{42u}));
    CHECK(lookup[0u] == 42u);
}

TEST_CASE("a lookup with some entries", "[linaer_search]") {
    constexpr auto lookup =
        LS::make(CX_VALUE(lookup::input<std::uint32_t, std::uint32_t, 2>{
            11u, std::array{lookup::entry{1u, 17u}, lookup::entry{2u, 42u}}}));
    CHECK(lookup[0u] == 11u);
    CHECK(lookup[1u] == 17u);
    CHECK(lookup[2u] == 42u);
}

TEST_CASE("a lookup with non-integer values", "[linear_search]") {
    constexpr auto lookup =
        LS::make(CX_VALUE(lookup::input<std::uint32_t, float, 2>{
            3.14f,
            std::array{lookup::entry{1u, 17.0f}, lookup::entry{2u, 42.0f}}}));
    CHECK(lookup[0u] == 3.14f);
    CHECK(lookup[1u] == 17.0f);
    CHECK(lookup[2u] == 42.0f);
}
