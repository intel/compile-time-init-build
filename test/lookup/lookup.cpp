#include "lookup/lookup.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <msg/detail/bitset.hpp>

// FIXME: need to test all the different lookup strategy types

namespace lookup {

TEST_CASE("a lookup with no entries", "[lookup]") {
    constexpr auto lookup =
        make<input<uint32_t, uint32_t, 0>>();

    REQUIRE(lookup[0] == 0);
    REQUIRE(lookup[42] == 0);
}

TEST_CASE("a lookup with no entries and non-zero default", "[lookup]") {
    constexpr auto lookup =
        make<input<uint32_t, uint32_t, 5>>();

    REQUIRE(lookup[0] == 5);
    REQUIRE(lookup[42] == 5);
}

TEST_CASE("a linear lookup with non-integer values", "[lookup]") {
    using msg::detail::bitset;

    constexpr auto lookup =
        linear_search_lookup<25>::make<
            input<uint32_t, bitset<32>,
                 bitset<32>{},
                 entry{42u, bitset<32>{0}},
                 entry{89u, bitset<32>{12, 3}}
             >
        >();

    REQUIRE(lookup[0] == bitset<32>{});
    REQUIRE(lookup[42] == bitset<32>{0});
    REQUIRE(lookup[89] == bitset<32>{12, 3});
}


TEST_CASE("a lookup with some entries", "[lookup]") {
    constexpr auto lookup =
        make<
            input<uint32_t, uint32_t, 0,
               entry{0u, 13u},
               entry{6u, 42u},
               entry{89u, 10u}
            >
        >();

    REQUIRE(lookup[0] == 13);
    REQUIRE(lookup[1] == 0);
    REQUIRE(lookup[5] == 0);
    REQUIRE(lookup[6] == 42);
    REQUIRE(lookup[7] == 0);
    REQUIRE(lookup[88] == 0);
    REQUIRE(lookup[89] == 10);
    REQUIRE(lookup[90] == 0);
}

TEST_CASE("a lookup with some entries and non-zero default", "[lookup]") {
    constexpr auto lookup =
        make<
            input<uint32_t, uint32_t, 5,
               entry{1u, 13u},
               entry{6u, 42u},
               entry{89u, 10u}
            >
        >();

    REQUIRE(lookup[0] == 5);
    REQUIRE(lookup[1] == 13);
    REQUIRE(lookup[5] == 5);
    REQUIRE(lookup[6] == 42);
    REQUIRE(lookup[7] == 5);
    REQUIRE(lookup[88] == 5);
    REQUIRE(lookup[89] == 10);
    REQUIRE(lookup[90] == 5);
}

TEST_CASE("a lookup with some sequential entries", "[lookup]") {
    constexpr auto lookup =
        make<
            input<uint32_t, uint32_t, 1,
                entry{0u, 13u},
                entry{1u, 42u},
                entry{2u, 10u},
                entry{3u, 76u},
                entry{4u, 25u},
                entry{5u, 82u},
                entry{6u, 18u},
                entry{7u, 87u},
                entry{8u, 55u},
                entry{9u, 11u}
            >
        >();

    REQUIRE(lookup[0] == 13);
    REQUIRE(lookup[1] == 42);
    REQUIRE(lookup[2] == 10);
    REQUIRE(lookup[3] == 76);
    REQUIRE(lookup[4] == 25);
    REQUIRE(lookup[5] == 82);
    REQUIRE(lookup[6] == 18);
    REQUIRE(lookup[7] == 87);
    REQUIRE(lookup[8] == 55);
    REQUIRE(lookup[9] == 11);
    REQUIRE(lookup[10] == 1);
    REQUIRE(lookup[11] == 1);
    REQUIRE(lookup[12] == 1);
    REQUIRE(lookup[13] == 1);
    REQUIRE(lookup[14] == 1);
    REQUIRE(lookup[15] == 1);
    REQUIRE(lookup[4000000000u] == 1);
}


}
