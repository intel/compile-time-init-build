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
