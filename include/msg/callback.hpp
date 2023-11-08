#pragma once

#include <log/log.hpp>
#include <match/ops.hpp>
#include <msg/detail/func_traits.hpp>
#include <msg/message.hpp>

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

template <typename T>
concept not_nullptr = not std::is_null_pointer_v<T>;

template <stdx::ct_string, typename...> struct callback;

template <typename...> struct extra_callback_args {};

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <stdx::ct_string Name, typename MsgDefn, typename... ExtraCallbackArgs,
          match::matcher M, detail::not_nullptr... Callables>
struct callback<Name, MsgDefn, extra_callback_args<ExtraCallbackArgs...>, M,
                Callables...> {
  private:
    decltype(std::declval<M>() and typename MsgDefn::matcher_t{}) matcher;
    stdx::tuple<Callables...> callbacks;

    template <typename Msg>
    void dispatch(Msg const &m, ExtraCallbackArgs const &...args) const {
        stdx::for_each(
            [&](auto const &cb) {
                detail::dispatch_single_callable(cb, m, args...);
            },
            callbacks);
    }

  public:
    template <typename... CBs>
    constexpr explicit callback(M const &m, CBs &&...cbs)
        : matcher{m and typename MsgDefn::matcher_t{}},
          callbacks{std::forward<CBs>(cbs)...} {}

    template <typename Msg>
    [[nodiscard]] auto is_match(Msg const &m) const -> bool {
        return matcher(m);
    }

    template <typename Msg>
    [[nodiscard]] auto handle(Msg const &m,
                              ExtraCallbackArgs const &...args) const -> bool {
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
};
} // namespace detail

template <stdx::ct_string Name, typename MsgDefn, typename... ExtraCallbackArgs>
constexpr auto callback = []<match::matcher M, typename... CBs>(
                              M const &m, CBs &&...callbacks) {
    return detail::callback<Name, MsgDefn,
                            detail::extra_callback_args<ExtraCallbackArgs...>,
                            M, std::remove_cvref_t<CBs>...>{
        m, std::forward<CBs>(callbacks)...};
};
} // namespace msg
