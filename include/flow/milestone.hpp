#pragma once

#include <flow/common.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>

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
template <stdx::ct_string Name>
[[nodiscard]] constexpr auto action(FunctionPtr f) -> node {
    return {.run = f, .log_name = [] {
                CIB_TRACE("flow.action({})",
                          stdx::ct_string_to_type<Name, sc::string_constant>());
            }};
}

/**
 * @return
 *      Node with no associated action.
 */
template <stdx::ct_string Name>
[[nodiscard]] constexpr auto milestone() -> node {
    return {.log_name = [] {
        CIB_TRACE("flow.milestone({})",
                  stdx::ct_string_to_type<Name, sc::string_constant>());
    }};
}
} // namespace flow
