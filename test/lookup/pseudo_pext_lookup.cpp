#include <lookup/input.hpp>
#include <lookup/pseudo_pext_lookup.hpp>

#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("lookup with some entries", "[pseudo pext lookup]") {
    constexpr auto lookup =
        lookup::pseudo_pext_lookup::make(CX_VALUE(lookup::input{
            0, std::array{lookup::entry{54u, 1}, lookup::entry{324u, 2},
                          lookup::entry{64u, 3}}}));

    CHECK(lookup[0] == 0);
    CHECK(lookup[54] == 1);
    CHECK(lookup[324] == 2);
    CHECK(lookup[64] == 3);
}

TEST_CASE("lookup with no entries", "[pseudo pext lookup]") {
    constexpr auto lookup =
        lookup::pseudo_pext_lookup::make(CX_VALUE(lookup::input<uint32_t>{0}));

    CHECK(lookup[0] == 0);
    CHECK(lookup[54] == 0);
    CHECK(lookup[324] == 0);
    CHECK(lookup[64] == 0);
}

TEST_CASE("lookup with non-integral values", "[pseudo pext lookup]") {
    constexpr auto lookup =
        lookup::pseudo_pext_lookup::make(CX_VALUE(lookup::input{
            0.0, std::array{lookup::entry{54u, 3.4}, lookup::entry{324u, 5.2},
                            lookup::entry{64u, 8.9}, lookup::entry{234u, 3.1},
                            lookup::entry{91u, 0.41}}}));

    CHECK(lookup[0] == 0.0);
    CHECK(lookup[54] == 3.4);
    CHECK(lookup[324] == 5.2);
    CHECK(lookup[64] == 8.9);
    CHECK(lookup[234] == 3.1);
    CHECK(lookup[91] == 0.41);
}

TEST_CASE("lookup with uint8_t entries", "[pseudo pext lookup]") {
    constexpr auto lookup = lookup::pseudo_pext_lookup::make(CX_VALUE(
        lookup::input{0, std::array{lookup::entry<uint8_t, int8_t>{54u, 1},
                                    lookup::entry<uint8_t, int8_t>{124u, 2},
                                    lookup::entry<uint8_t, int8_t>{64u, 3}}}));

    CHECK(lookup[0u] == 0);
    CHECK(lookup[54u] == 1);
    CHECK(lookup[124u] == 2);
    CHECK(lookup[64u] == 3);
}

enum class some_key_t : uint16_t {
    ALPHA = 0u,
    BETA = 1u,
    KAPPA = 2u,
    GAMMA = 3u
};

TEST_CASE("lookup with scoped enum entries", "[pseudo pext lookup]") {
    constexpr auto lookup =
        lookup::pseudo_pext_lookup::make(CX_VALUE(lookup::input{
            0, std::array{
                   lookup::entry<some_key_t, int8_t>{some_key_t::ALPHA, 54},
                   lookup::entry<some_key_t, int8_t>{some_key_t::BETA, 23},
                   lookup::entry<some_key_t, int8_t>{some_key_t::KAPPA, 87},
                   lookup::entry<some_key_t, int8_t>{some_key_t::GAMMA, 4}}}));

    CHECK(lookup[some_key_t::ALPHA] == 54);
    CHECK(lookup[some_key_t::BETA] == 23);
    CHECK(lookup[some_key_t::KAPPA] == 87);
    CHECK(lookup[some_key_t::GAMMA] == 4);
}
