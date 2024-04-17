#pragma once

#include <lookup/direct_array_lookup.hpp>
#include <lookup/fast_hash_lookup.hpp>
#include <lookup/hash_ops.hpp>
#include <lookup/linear_search_lookup.hpp>
#include <lookup/pseudo_pext_lookup.hpp>
#include <lookup/strategies.hpp>

namespace lookup::strategy {
// benchmarked on ARC HS4X HSDK dev board
using arc_cpu = strategies<
    // 4.1 - 6.39 cycles unrolled 4x
    linear_search_lookup<3>,

    // 6.51 cycles unrolled 4x
    direct_array_lookup<25>,

    // 9.39 cycles unrolled 4x
    linear_search_lookup<7>,

    pseudo_pext_lookup>;
} // namespace lookup::strategy
