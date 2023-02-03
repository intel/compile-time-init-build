#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <container/Vector.hpp>
#include <log/log.hpp>
#include <msg/handler_interface.hpp>
#include <msg/message.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace msg {
template <typename T>
using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <typename...> struct callback_types {};

template <typename...> struct extra_callback_args {};

template <typename BaseMsgT, typename ExtraCallbackArgsListT,
          typename NameTypeT, typename MatchMsgTypeT,
          typename CallbackTypeListT>
struct callback_impl;

template <typename CallableT> struct func_args {
    using msg_type = typename func_args<
        decltype(&remove_cvref_t<CallableT>::operator())>::msg_type;
    using type = typename func_args<
        decltype(&remove_cvref_t<CallableT>::operator())>::type;
};

template <typename DataIterableT, typename... ArgTs>
struct func_args<void(DataIterableT, ArgTs...)> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename DataIterableT, typename... ArgTs>
struct func_args<void(DataIterableT, ArgTs...) const> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename DataIterableT, typename T, typename... ArgTs>
struct func_args<void (T::*)(DataIterableT, ArgTs...)> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename DataIterableT, typename T, typename... ArgTs>
struct func_args<void (T::*)(DataIterableT, ArgTs...) const> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename CallableT>
using func_args_t = typename func_args<CallableT>::type;

template <typename CallableT> constexpr func_args_t<CallableT> func_args_v{};

template <typename CallbackType>
using msg_type_t = typename func_args<CallbackType>::msg_type;

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
        func_args_v<CallableT>);

    required_args_tuple.apply([&](auto const &...requiredArgs) {
        using MsgType = msg_type_t<decltype(callable)>;
        MsgType const msg{data};

        if (msg.isValid()) {
            callable(msg, requiredArgs...);
        }
    });
}

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
                using MsgType = msg_type_t<decltype(callback)>;
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

template <typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct handler : handler_interface<BaseMsgT, ExtraCallbackArgsT...> {
    CallbacksT callbacks{};

    constexpr explicit handler(CallbacksT new_callbacks)
        : callbacks{new_callbacks} {}

    auto is_match(BaseMsgT const &msg) const -> bool final {
        return callbacks.fold_left(false, [&](bool state, auto &callback) {
            return state || callback.is_match(msg);
        });
    }

    void handle(BaseMsgT const &msg, ExtraCallbackArgsT... args) const final {
        bool const found_valid_callback =
            callbacks.fold_left(false, [&](bool state, auto &callback) {
                return state || callback.handle(msg, args...);
            });

        if (!found_valid_callback) {
            CIB_ERROR("None of the registered callbacks claimed this message:");

            callbacks.for_each(
                [&](auto &callback) { callback.log_mismatch(msg); });
        }
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
