#pragma once

#include <log/log.hpp>
#include <match/ops.hpp>
#include <msg/detail/func_traits.hpp>
#include <msg/message.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace detail {
template <typename Callable>
void dispatch_single_callable(Callable const &callable, auto const &msg,
                              auto const &...args) {
    auto const provided_args_tuple = stdx::make_tuple(args...);
    auto const required_args_tuple = stdx::transform(
        [&]<typename Arg>(Arg const &) {
            return get<Arg>(provided_args_tuple);
        },
        detail::func_args_v<Callable>);

    required_args_tuple.apply(
        [&](auto const &...requiredArgs) { callable(msg, requiredArgs...); });
}

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <stdx::ct_string Name, match::matcher M, stdx::callable F,
          typename... ExtraArgs>
struct callback {
  private:
    template <typename Msg>
    void dispatch(Msg const &m, ExtraArgs const &...args) const {
        detail::dispatch_single_callable(callable, m, args...);
    }

  public:
    template <typename Msg>
    [[nodiscard]] auto is_match(Msg const &m) const -> bool {
        return matcher(m);
    }

    template <typename Msg>
    [[nodiscard]] auto handle(Msg const &m, ExtraArgs const &...args) const
        -> bool {
        if (matcher(m)) {
            CIB_INFO("Incoming message matched [{}], because [{}], executing "
                     "callback",
                     stdx::ct_string_to_type<Name, sc::string_constant>(),
                     matcher.describe());
            dispatch(m, args...);
            return true;
        }
        return false;
    }

    template <typename Msg> auto log_mismatch(Msg const &m) const -> void {
        CIB_INFO("    {} - F:({})",
                 stdx::ct_string_to_type<Name, sc::string_constant>(),
                 matcher.describe_match(m));
    }

    using matcher_t = M;
    using callable_t = F;

    template <match::matcher NewM>
    using rebind_matcher = callback<Name, NewM, F, ExtraArgs...>;

    constexpr static auto name = Name;
    [[no_unique_address]] M matcher;
    [[no_unique_address]] F callable;
};
} // namespace detail

template <stdx::ct_string Name, typename... ExtraArgs>
constexpr auto callback =
    []<match::matcher M, stdx::callable F>(M const &m, F &&f) {
        return detail::callback<Name, M, std::remove_cvref_t<F>, ExtraArgs...>{
            m, std::forward<F>(f)};
    };
} // namespace msg
