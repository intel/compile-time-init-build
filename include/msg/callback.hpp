#pragma once

#include <cib/tuple.hpp>
#include <msg/detail/func_traits.hpp>
#include <msg/message.hpp>

namespace msg {

template <typename CallableT, typename DataIterableT,
          typename... ExtraCallbackArgsT>
void dispatch_single_callable(CallableT const &callable,
                              DataIterableT const &data,
                              ExtraCallbackArgsT const &...args) {
    auto const provided_args_tuple = cib::make_tuple(args...);
    auto const required_args_tuple = cib::transform(
        [&](auto requiredArg) {
            using RequiredArgType = decltype(requiredArg);
            return cib::get<RequiredArgType>(provided_args_tuple);
        },
        detail::func_args_v<CallableT>);

    required_args_tuple.apply([&](auto const &...requiredArgs) {
        using MsgType = detail::msg_type_t<decltype(callable)>;
        MsgType const msg{data};

        if (msg.isValid()) {
            callable(msg, requiredArgs...);
        }
    });
}

template <typename...> struct callback_impl;

template <typename...> struct extra_callback_args {};

namespace detail {
template <typename T>
concept not_nullptr = not
std::is_null_pointer_v<T>;
} // namespace detail

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <typename BaseMsgT, typename... ExtraCallbackArgsT, typename NameTypeT,
          typename MatchMsgTypeT, detail::not_nullptr... CallableTypesT>
struct callback_impl<BaseMsgT, extra_callback_args<ExtraCallbackArgsT...>,
                     NameTypeT, MatchMsgTypeT, CallableTypesT...> {
  private:
    constexpr static NameTypeT name{};

    MatchMsgTypeT match_msg;
    cib::tuple<CallableTypesT...> callbacks;

    template <typename DataIterableType>
    void dispatch(DataIterableType const &data,
                  ExtraCallbackArgsT const &...args) const {
        cib::for_each(
            [&](auto const &callback) {
                dispatch_single_callable(callback, data, args...);
            },
            callbacks);
    }

    [[nodiscard]] constexpr auto match_any_callback() const {
        auto const matchers = cib::transform(
            [&](auto callback) {
                using MsgType = detail::msg_type_t<decltype(callback)>;
                return is_valid_msg<MsgType>(match::always<true>);
            },
            callbacks);

        return matchers.apply(
            [](auto... matchersPack) { return match::any(matchersPack...); });
    }

  public:
    template <typename... CBs>
    constexpr explicit callback_impl(MatchMsgTypeT const &msg, CBs &&...cbs)
        : match_msg(msg), callbacks{std::forward<CBs>(cbs)...} {}

    [[nodiscard]] auto is_match(BaseMsgT const &msg) const -> bool {
        return match::all(match_msg, match_any_callback())(msg);
    }

    [[nodiscard]] auto handle(BaseMsgT const &msg,
                              ExtraCallbackArgsT const &...args) const -> bool {
        auto match_handler = match::all(match_msg, match_any_callback());

        if (match_handler(msg)) {
            CIB_INFO("Incoming message matched [{}], because [{}], executing "
                     "callback",
                     name, match_handler.describe());

            dispatch(msg.data, args...);

            return true;
        }

        return false;
    }

    auto log_mismatch(BaseMsgT const &msg) const -> void {
        CIB_INFO(
            "    {} - F:({})", name,
            match::all(match_msg, match_any_callback()).describe_match(msg));
    }
};

template <typename BaseMsgT, typename... ExtraCallbackArgsT>
constexpr auto callback = []<typename Name, typename MatchMsg, typename... CBs>(
                              Name, MatchMsg match_msg, CBs &&...callbacks) {
    return callback_impl<BaseMsgT, extra_callback_args<ExtraCallbackArgsT...>,
                         Name, MatchMsg, std::remove_cvref_t<CBs>...>{
        match_msg, std::forward<CBs>(callbacks)...};
};

} // namespace msg
