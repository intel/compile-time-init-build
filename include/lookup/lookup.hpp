#pragma once

#include <lookup/input.hpp>
#include <lookup/linear_search_lookup.hpp>
#include <lookup/pseudo_pext_lookup.hpp>
#include <lookup/strategies.hpp>

#include <stdx/compiler.hpp>

namespace lookup {
[[nodiscard]] CONSTEVAL static auto make(compile_time auto input) {
    return strategies<linear_search_lookup<4>,
                      pseudo_pext_lookup<>>::make(input);
}
} // namespace lookup
