#include <lookup/direct_array_lookup.hpp>
#include <lookup/input.hpp>
#include <lookup/linear_search_lookup.hpp>
#include <lookup/lookup.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/ct_conversions.hpp>
#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using S = lookup::strategies<lookup::direct_array_lookup<50>,
                             lookup::linear_search_lookup<3>>;
}

TEST_CASE("succeed on first strategy", "[direct array]") {
    constexpr auto lookup = S::make(CX_VALUE(
        lookup::input{0u, std::array{lookup::entry{1u, 1}, lookup::entry{2u, 2},
                                     lookup::entry{3u, 3}}}));
    static_assert(stdx::type_as_string<decltype(lookup)>().find(
                      "direct_array") != std::string_view::npos);
}

TEST_CASE("fail on first strategy, succeed on later", "[direct array]") {
    constexpr auto lookup = S::make(CX_VALUE(
        lookup::input{0, std::array{lookup::entry{10, 1}, lookup::entry{20, 2},
                                    lookup::entry{30, 3}}}));
    static_assert(stdx::type_as_string<decltype(lookup)>().find(
                      "linear_search") != std::string_view::npos);
}

TEST_CASE("fail on all strategies", "[direct array]") {
    constexpr auto lookup = S::make(CX_VALUE(lookup::input{
        0, std::array{lookup::entry{10, 1}, lookup::entry{20, 2},
                      lookup::entry{30, 3}, lookup::entry{40, 4}}}));
    static_assert(lookup::strategy_failed(lookup));
}
