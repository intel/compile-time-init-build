#pragma once

#include <log/log.hpp>
#include <match/ops.hpp>
#include <msg/detail/func_traits.hpp>
#include <msg/message.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace detail {
template <typename Callable>
void dispatch_single_callable(Callable const &callable, auto const &base_msg,
                              auto const &...args) {
    auto const provided_args_tuple = stdx::make_tuple(args...);
    auto const required_args_tuple = stdx::transform(
        [&]<typename Arg>(Arg const &) {
            return get<Arg>(provided_args_tuple);
        },
        detail::func_args_v<Callable>);

    required_args_tuple.apply([&](auto const &...requiredArgs) {
        using MsgType = detail::msg_type_t<Callable>;
        MsgType const msg{base_msg};

        if (typename MsgType::matcher_t{}(msg)) {
            callable(msg, requiredArgs...);
        }
    });
}

template <typename T>
concept not_nullptr = not std::is_null_pointer_v<T>;
} // namespace detail

template <typename...> struct callback_impl;

template <typename...> struct extra_callback_args {};

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <typename BaseMsg, typename... ExtraCallbackArgs, typename Name,
          match::matcher M, detail::not_nullptr... Callables>
struct callback_impl<BaseMsg, extra_callback_args<ExtraCallbackArgs...>, Name,
                     M, Callables...> {
  private:
    constexpr static Name name{};

    [[nodiscard]] constexpr static auto match_any_callback() {
        return match::any(
            typename detail::msg_type_t<Callables>::matcher_t{}...);
    }

    decltype(std::declval<M>() and match_any_callback()) matcher;
    stdx::tuple<Callables...> callbacks;

    void dispatch(BaseMsg const &msg, ExtraCallbackArgs const &...args) const {
        stdx::for_each(
            [&](auto const &callback) {
                detail::dispatch_single_callable(callback, msg, args...);
            },
            callbacks);
    }

  public:
    template <typename... CBs>
    constexpr explicit callback_impl(M const &m, CBs &&...cbs)
        : matcher{m and match_any_callback()},
          callbacks{std::forward<CBs>(cbs)...} {}

    [[nodiscard]] auto is_match(BaseMsg const &msg) const -> bool {
        return matcher(msg);
    }

    [[nodiscard]] auto handle(BaseMsg const &msg,
                              ExtraCallbackArgs const &...args) const -> bool {
        if (matcher(msg)) {
            CIB_INFO("Incoming message matched [{}], because [{}], executing "
                     "callback",
                     name, matcher.describe());

            dispatch(msg, args...);
            return true;
        }
        return false;
    }

    auto log_mismatch(BaseMsg const &msg) const -> void {
        CIB_INFO("    {} - F:({})", name, matcher.describe_match(msg));
    }
};

template <typename BaseMsg, typename... ExtraCallbackArgs>
constexpr auto callback = []<typename Name, match::matcher M, typename... CBs>(
                              Name, M const &m, CBs &&...callbacks) {
    return callback_impl<BaseMsg, extra_callback_args<ExtraCallbackArgs...>,
                         Name, M, std::remove_cvref_t<CBs>...>{
        m, std::forward<CBs>(callbacks)...};
};
} // namespace msg
