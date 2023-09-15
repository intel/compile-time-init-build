#include "cx_value.hpp"

#include <lookup/fast_hash_lookup.hpp>
#include <lookup/hash_ops.hpp>
#include <lookup/input.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
struct bad_hash_op {
    [[nodiscard]] constexpr static inline auto calc(auto) { return 0; }
};

struct evens_hash_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return (x / 2) * 2;
    }
};

using fail_t = lookup::fast_hash_lookup<50, 2, bad_hash_op>;
using same_t = lookup::fast_hash_lookup<50, 1, lookup::id_op>;
using even_t = lookup::fast_hash_lookup<50, 2, evens_hash_op>;
} // namespace

TEST_CASE("failing hash", "[fast hash]") {
    constexpr auto lookup = fail_t::make(CX_VALUE(lookup::input{
        0u, std::array{lookup::entry{10u, 1}, lookup::entry{20u, 2},
                       lookup::entry{30u, 3}}}));
    static_assert(lookup::strategy_failed(lookup));
}

TEST_CASE("a lookup with no entries", "[fast hash]") {
    constexpr auto lookup = same_t::make(CX_VALUE(lookup::input{42u}));
    CHECK(lookup[0u] == 42u);
}

TEST_CASE("a lookup with some entries", "[fast hash]") {
    constexpr auto lookup = same_t::make(CX_VALUE(lookup::input{
        0u, std::array{lookup::entry{1u, 42u}, lookup::entry{2u, 17u}}}));
    CHECK(lookup[0u] == 0u);
    CHECK(lookup[1u] == 42u);
    CHECK(lookup[2u] == 17u);
}

TEST_CASE("a lookup with some entries that clash", "[fast hash]") {
    constexpr auto lookup = even_t::make(CX_VALUE(lookup::input{
        0u, std::array{lookup::entry{1u, 42u}, lookup::entry{2u, 17u}}}));
    CHECK(lookup[0u] == 0u);
    CHECK(lookup[1u] == 42u);
    CHECK(lookup[2u] == 17u);
}
