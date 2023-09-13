#pragma once
#include <lookup/strategy/arc_cpu.hpp>

#include <stdx/compiler.hpp>

namespace lookup {
// TODO: need a good way to make this extendable
template <typename InputValues> [[nodiscard]] CONSTEVAL static auto make() {
    return lookup::strategy::arc_cpu::make<InputValues>();
}
} // namespace lookup
