#pragma once

#include <cib/tuple.hpp>

#include <type_traits>

namespace msg::detail {

template <typename T>
using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

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

template <typename DataIterableT, typename... ArgTs>
struct func_args<void(DataIterableT, ArgTs...) noexcept> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename DataIterableT, typename... ArgTs>
struct func_args<void(DataIterableT, ArgTs...) const noexcept> {
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

template <typename DataIterableT, typename T, typename... ArgTs>
struct func_args<void (T::*)(DataIterableT, ArgTs...) noexcept> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename DataIterableT, typename T, typename... ArgTs>
struct func_args<void (T::*)(DataIterableT, ArgTs...) const noexcept> {
    using msg_type = remove_cvref_t<DataIterableT>;
    using type = cib::tuple<ArgTs...>;
};

template <typename CallableT>
using func_args_t = typename func_args<CallableT>::type;

template <typename CallableT> constexpr func_args_t<CallableT> func_args_v{};

template <typename CallbackType>
using msg_type_t = typename func_args<CallbackType>::msg_type;
} // namespace msg::detail
