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
    auto const provided_args_tuple =
        cib::make_tuple(cib::self_type_index, args...);
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

template <typename BaseMsgT, typename ExtraCallbackArgsListT,
          typename NameTypeT, typename MatchMsgTypeT,
          typename CallbackTypeListT>
struct callback_impl;

template <typename...> struct callback_types {};

template <typename...> struct extra_callback_args {};

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <typename BaseMsgT, typename... ExtraCallbackArgsT, typename NameTypeT,
          typename MatchMsgTypeT, typename... CallableTypesT>
struct callback_impl<BaseMsgT, extra_callback_args<ExtraCallbackArgsT...>,
                     NameTypeT, MatchMsgTypeT,
                     callback_types<CallableTypesT...>> {
  private:
    constexpr static NameTypeT name{};

    MatchMsgTypeT match_msg;
    cib::tuple<CallableTypesT...> callbacks;

    template <typename DataIterableType>
    void dispatch(DataIterableType const &data,
                  ExtraCallbackArgsT const &...args) const {
        callbacks.for_each([&](auto callback) {
            dispatch_single_callable(callback, data, args...);
        });
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
    constexpr explicit callback_impl(MatchMsgTypeT const &msg,
                                     CallableTypesT const &...callback_args)
        : match_msg(msg), callbacks(cib::make_tuple(callback_args...)) {
        callbacks.for_each([](auto callback) {
            static_assert(
                !std::is_same<decltype(callback), std::nullptr_t>::value,
                "Function pointer specified for callback can't be null");
        });
    }

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
auto callback = [](auto name, auto match_msg, auto... callbacks) {
    return callback_impl<BaseMsgT, extra_callback_args<ExtraCallbackArgsT...>,
                         decltype(name), decltype(match_msg),
                         callback_types<decltype(callbacks)...>>{match_msg,
                                                                 callbacks...};
};

} // namespace msg
