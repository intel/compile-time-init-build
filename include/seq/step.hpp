#pragma once

#include <flow/detail/dependency.hpp>
#include <flow/detail/parallel.hpp>

#include <cstddef>
#include <cstdint>

namespace seq {
enum class status { NOT_DONE = 0, DONE = 1 };

using func_ptr = auto(*)() -> status;
using log_func_ptr = auto(*)() -> void;

class step_base {
  private:
    func_ptr _forward_ptr{};
    func_ptr _backward_ptr{};
    log_func_ptr log_name{};

#if defined(__GNUC__) && __GNUC__ < 12
    uint64_t hash{};
#endif

    template <std::size_t NumSteps> friend struct impl;

  public:
    template <typename Name>
    constexpr step_base([[maybe_unused]] Name name, func_ptr forward_ptr,
                        func_ptr backward_ptr)
        : _forward_ptr{forward_ptr}, _backward_ptr{backward_ptr},
          log_name{[]() { CIB_TRACE("seq.step({})", Name{}); }}

#if defined(__GNUC__) && __GNUC__ < 12
          ,
          hash{name.hash()}
#endif
    {
    }

    constexpr step_base() = default;

    [[nodiscard]] constexpr friend auto operator==(step_base const &lhs,
                                                   step_base const &rhs)
        -> bool {
#if defined(__GNUC__) && __GNUC__ < 12
        return lhs.hash == rhs.hash;
#else
        return (lhs._forward_ptr) == (rhs._forward_ptr) &&
               (lhs._backward_ptr) == (rhs._backward_ptr) &&
               (lhs.log_name) == (rhs.log_name);
#endif
    }

    constexpr void forward() const { _forward_ptr(); }
    constexpr void backward() const { _backward_ptr(); }
};

template <typename LhsT, typename RhsT>
[[nodiscard]] constexpr auto operator>>(LhsT const &lhs, RhsT const &rhs)
    -> flow::detail::dependency<step_base, LhsT, RhsT> {
    return {lhs, rhs};
}

template <typename LhsT, typename RhsT>
[[nodiscard]] constexpr auto operator&&(LhsT const &lhs, RhsT const &rhs)
    -> flow::detail::parallel<step_base, LhsT, RhsT> {
    return {lhs, rhs};
}

/**
 * @param f
 *      The function pointer to execute.
 *
 * @return
 *      New action that will execute the given function pointer, f.
 */
template <typename NameType>
[[nodiscard]] constexpr auto step(NameType name, func_ptr forward,
                                  func_ptr backward) -> step_base {
    return {name, forward, backward};
}
} // namespace seq
