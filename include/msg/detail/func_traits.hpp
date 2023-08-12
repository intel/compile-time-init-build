#pragma once

#include <cib/tuple.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace msg::detail {
template <typename...> struct function_type;

template <typename R, typename Msg, typename... Args>
struct function_type<std::function<R(Msg, Args...)>> {
    using msg_type = std::remove_cvref_t<Msg>;
    using args_type = cib::tuple<Args...>;
};

template <typename F>
using func_t = function_type<decltype(std::function{std::declval<F>()})>;

template <typename Callable>
using func_args_t = typename func_t<CallableT>::args_type;

template <typename Callable> constexpr func_args_t<CallableT> func_args_v{};

template <typename Callable>
using msg_type_t = typename func_t<Callable>::msg_type;
} // namespace msg::detail
