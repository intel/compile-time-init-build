#pragma once

#include <flow/detail/dependency.hpp>
#include <flow/detail/parallel.hpp>

namespace flow {
using FunctionPtr = auto(*)() -> void;

class milestone_base {
  private:
    FunctionPtr run{};
    FunctionPtr log_name{};

    template <typename Name, std::size_t NumSteps> friend class impl;

  public:
    template <typename Name>
    constexpr milestone_base([[maybe_unused]] Name name, FunctionPtr run_ptr)
        : run{run_ptr}, log_name{[]() {
              CIB_TRACE("flow.milestone({})", Name{});
          }} {}

    constexpr milestone_base() = default;

    constexpr void operator()() const { run(); }

  private:
    [[nodiscard]] friend constexpr auto operator==(milestone_base const &lhs,
                                                   milestone_base const &rhs)
        -> bool = default;
};

template <typename LhsT, typename RhsT>
[[nodiscard]] constexpr auto operator>>(LhsT const &lhs, RhsT const &rhs) {
    return detail::dependency<milestone_base, LhsT, RhsT>{lhs, rhs};
}

template <typename LhsT, typename RhsT>
[[nodiscard]] constexpr auto operator&&(LhsT const &lhs, RhsT const &rhs) {
    return detail::parallel<milestone_base, LhsT, RhsT>{lhs, rhs};
}

/**
 * @param f
 *      The function pointer to execute.
 *
 * @return
 *      New action that will execute the given function pointer, f.
 */
template <typename NameType>
[[nodiscard]] constexpr auto action(NameType name, FunctionPtr f) {
    return milestone_base{name, f};
}

/**
 * @return
 *      New milestone_base with no associated action.
 */
template <typename NameType>
[[nodiscard]] constexpr auto milestone(NameType name) {
    return milestone_base{name, []() {}};
}
} // namespace flow
