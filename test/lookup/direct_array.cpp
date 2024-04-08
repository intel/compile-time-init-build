#include <lookup/direct_array_lookup.hpp>
#include <lookup/input.hpp>

#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using DA = lookup::direct_array_lookup<50>;
}

TEST_CASE("a sparser lookup than allowed", "[direct array]") {
    constexpr auto lookup = DA::make(CX_VALUE(
        lookup::input{0, std::array{lookup::entry{10, 1}, lookup::entry{20, 2},
                                    lookup::entry{30, 3}}}));
    static_assert(lookup::strategy_failed(lookup));
}

TEST_CASE("a lookup with no entries", "[direct array]") {
    constexpr auto lookup = DA::make(CX_VALUE(lookup::input{0}));
    static_assert(lookup::strategy_failed(lookup));
}

TEST_CASE("a lookup with some entries", "[direct array]") {
    constexpr auto lookup = DA::make(CX_VALUE(lookup::input{
        11u, std::array{lookup::entry{1u, 42u}, lookup::entry{2u, 17u}}}));
    CHECK(lookup[0u] == 11u);
    CHECK(lookup[1u] == 42u);
    CHECK(lookup[2u] == 17u);
}

TEST_CASE("a lookup with non-integer entries", "[direct array]") {
    constexpr auto lookup = DA::make(CX_VALUE(lookup::input{
        0.0f, std::array{lookup::entry{1u, 1.0f}, lookup::entry{2u, 2.0f}}}));
    static_assert(lookup::strategy_failed(lookup));
}
