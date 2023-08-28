#pragma once

#include <log/log.hpp>

namespace seq {
enum struct status { NOT_DONE = 0, DONE = 1 };

using func_ptr = auto (*)() -> status;
using log_func_ptr = auto (*)() -> void;

struct step_base {
    using is_node = void;

    func_ptr forward_ptr{};
    func_ptr backward_ptr{};
    log_func_ptr log_name{};

  private:
    [[nodiscard]] constexpr friend auto operator==(step_base const &lhs,
                                                   step_base const &rhs)
        -> bool = default;
};

/**
 * @param forward
 *      The function pointer to execute on a forward step.
 * @param backward
 *      The function pointer to execute on a backward step.
 *
 * @return
 *      Step that will execute the given function pointers.
 */
template <typename Name>
[[nodiscard]] constexpr auto step(Name, func_ptr forward, func_ptr backward)
    -> step_base {
    return {forward, backward, [] { CIB_TRACE("seq.step({})", Name{}); }};
}
} // namespace seq
