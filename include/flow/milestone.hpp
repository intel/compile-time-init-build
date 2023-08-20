#pragma once

#include <flow/common.hpp>
#include <log/log.hpp>

namespace flow {
struct node {
    using is_node = void;

    FunctionPtr run{[] {}};
    FunctionPtr log_name{[] {}};

  private:
    [[nodiscard]] friend constexpr auto operator==(node const &lhs,
                                                   node const &rhs)
        -> bool = default;
};

/**
 * @param f
 *      The function pointer to execute.
 *
 * @return
 *      Node that will execute the given function pointer, f.
 */
template <typename Name>
[[nodiscard]] constexpr auto action(Name, FunctionPtr f) -> node {
    return {.run = f, .log_name = [] { CIB_TRACE("flow.action({})", Name{}); }};
}

/**
 * @return
 *      Node with no associated action.
 */
template <typename Name> [[nodiscard]] constexpr auto milestone(Name) -> node {
    return {.log_name = [] { CIB_TRACE("flow.milestone({})", Name{}); }};
}
} // namespace flow
