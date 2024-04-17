#define ANKERL_NANOBENCH_IMPLEMENT
#include <lookup/input.hpp>
#include <lookup/pseudo_pext_lookup.hpp>

#include <stdx/utility.hpp>

#include <mph>

#include <nanobench.h>

constexpr auto pseudo_pext_map =
    lookup::pseudo_pext_lookup::make(CX_VALUE(lookup::input{
        0, std::array{lookup::entry{54u, 91u}, lookup::entry{324u, 54u},
                      lookup::entry{64u, 324u}, lookup::entry{234u, 64u},
                      lookup::entry{91u, 234u}}}));

static_assert(pseudo_pext_map[12u] == 0u);
static_assert(pseudo_pext_map[54u] == 91u);
static_assert(pseudo_pext_map[324u] == 54u);
static_assert(pseudo_pext_map[64u] == 324u);
static_assert(pseudo_pext_map[234u] == 64u);
static_assert(pseudo_pext_map[91u] == 234u);

constexpr auto conditional_pext_policy = []<auto const... ts>(auto &&...args) {
    return mph::pext<7u, mph::conditional>{}.template operator()<ts...>(
        std::forward<decltype(args)>(args)...);
};

constexpr auto mph_map =
    mph::hash<std::array{mph::pair{54u, 91u}, mph::pair{324u, 54u},
                         mph::pair{64u, 324u}, mph::pair{234u, 64u},
                         mph::pair{91u, 234u}},
              conditional_pext_policy>;

static_assert(mph_map(12u) == false);
static_assert(*mph_map(54u) == 91u);
static_assert(*mph_map(324u) == 54u);
static_assert(*mph_map(64u) == 324u);
static_assert(*mph_map(234u) == 64u);
static_assert(*mph_map(91u) == 234u);

constexpr auto unconditional_pext_policy =
    []<auto const... ts>(auto &&...args) {
        return mph::pext<7u, mph::unconditional>{}.template operator()<ts...>(
            std::forward<decltype(args)>(args)...);
    };

constexpr auto unconditional_mph_map =
    mph::hash<std::array{mph::pair{54u, 91u}, mph::pair{324u, 54u},
                         mph::pair{64u, 324u}, mph::pair{234u, 64u},
                         mph::pair{91u, 234u}},
              unconditional_pext_policy>;

__attribute__((always_inline, flatten)) auto test(unsigned int k) {
    return pseudo_pext_map[k];
}

int main() {
    unsigned int k = 324u;
    ankerl::nanobench::Bench().minEpochIterations(20000000).run(
        "pseudo pext lookup", [&] {
            k = test(k);
            ankerl::nanobench::doNotOptimizeAway(k);
        });

    k = 324u;
    ankerl::nanobench::Bench().minEpochIterations(20000000).run(
        "conditional mph pext lookup", [&] {
            k = *mph_map(k);
            ankerl::nanobench::doNotOptimizeAway(k);
        });

    k = 324u;
    ankerl::nanobench::Bench().minEpochIterations(20000000).run(
        "unconditional mph pext lookup", [&] {
            k = unconditional_mph_map(k);
            ankerl::nanobench::doNotOptimizeAway(k);
        });
}
