#pragma once

#include <container/Vector.hpp>

#include <type_traits>

namespace seq {
class step_base;

enum class status { NOT_DONE = 0, DONE = 1 };

using func_ptr = status (*)();
} // namespace seq

namespace seq::detail {
template <typename NameType> void log_flow_step() {
    CIB_TRACE("seq.step({})", NameType{});
}
} // namespace seq::detail
