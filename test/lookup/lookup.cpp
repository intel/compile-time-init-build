#include "cx_value.hpp"

#include <lookup/entry.hpp>
#include <lookup/input.hpp>
#include <lookup/lookup.hpp>

#include <stdx/bitset.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

TEST_CASE("a lookup with no entries", "[lookup]") {
    constexpr auto lookup =
        lookup::make(CX_VALUE(lookup::input<std::uint32_t>{}));
    static_assert(lookup[0u] == 0u);
    static_assert(lookup[42u] == 0u);
}

TEST_CASE("a lookup with no entries and non-zero default", "[lookup]") {
    constexpr auto lookup =
        lookup::make(CX_VALUE(lookup::input<std::uint32_t>{5u}));
    static_assert(lookup[0u] == 5u);
    static_assert(lookup[42u] == 5u);
}

namespace {
template <auto N> using bitset = stdx::bitset<N, std::uint32_t>;
}

TEST_CASE("a linear lookup with non-integer values", "[lookup]") {
    constexpr auto lookup =
        lookup::linear_search_lookup<25>::make(CX_VALUE(lookup::input{
            bitset<32>{},
            std::array{
                lookup::entry{42, bitset<32>{stdx::place_bits, 0}},
                lookup::entry{89, bitset<32>{stdx::place_bits, 12, 3}}}}));

    static_assert(lookup[0] == bitset<32>{});
    static_assert(lookup[42] == bitset<32>{stdx::place_bits, 0});
    static_assert(lookup[89] == bitset<32>{stdx::place_bits, 12, 3});
}

TEST_CASE("a lookup with some entries", "[lookup]") {
    constexpr auto lookup = lookup::make(CX_VALUE(
        lookup::input{0, std::array{lookup::entry{0, 13}, lookup::entry{6, 42},
                                    lookup::entry{89, 10}}}));
    static_assert(lookup[0] == 13);
    static_assert(lookup[1] == 0);
    static_assert(lookup[5] == 0);
    static_assert(lookup[6] == 42);
    static_assert(lookup[7] == 0);
    static_assert(lookup[88] == 0);
    static_assert(lookup[89] == 10);
    static_assert(lookup[90] == 0);
}

TEST_CASE("a lookup with some entries and non-zero default", "[lookup]") {
    constexpr auto lookup = lookup::make(CX_VALUE(
        lookup::input{5, std::array{lookup::entry{1, 13}, lookup::entry{6, 42},
                                    lookup::entry{89, 10}}}));
    static_assert(lookup[0] == 5);
    static_assert(lookup[1] == 13);
    static_assert(lookup[5] == 5);
    static_assert(lookup[6] == 42);
    static_assert(lookup[7] == 5);
    static_assert(lookup[88] == 5);
    static_assert(lookup[89] == 10);
    static_assert(lookup[90] == 5);
}

TEST_CASE("a lookup with some sequential entries", "[lookup]") {
    constexpr auto lookup = lookup::make(CX_VALUE(lookup::input{
        1u, std::array{lookup::entry{0u, 13u}, lookup::entry{1u, 42u},
                       lookup::entry{2u, 10u}, lookup::entry{3u, 76u},
                       lookup::entry{4u, 25u}, lookup::entry{5u, 82u},
                       lookup::entry{6u, 18u}, lookup::entry{7u, 87u},
                       lookup::entry{8u, 55u}, lookup::entry{9u, 11u}}}));
    static_assert(lookup[0] == 13);
    static_assert(lookup[1] == 42);
    static_assert(lookup[2] == 10);
    static_assert(lookup[3] == 76);
    static_assert(lookup[4] == 25);
    static_assert(lookup[5] == 82);
    static_assert(lookup[6] == 18);
    static_assert(lookup[7] == 87);
    static_assert(lookup[8] == 55);
    static_assert(lookup[9] == 11);
    static_assert(lookup[10] == 1);
    static_assert(lookup[11] == 1);
    static_assert(lookup[12] == 1);
    static_assert(lookup[13] == 1);
    static_assert(lookup[14] == 1);
    static_assert(lookup[15] == 1);
    static_assert(lookup[4'000'000'000u] == 1);
}
