#pragma once
#include <lookup/input.hpp>
#include <lookup/strategy/arc_cpu.hpp>

#include <stdx/compiler.hpp>

namespace lookup {
// TODO: need a good way to make this extendable
[[nodiscard]] CONSTEVAL static auto make(compile_time auto input) {
    return lookup::strategy::arc_cpu::make(input);
}
} // namespace lookup
