#pragma once

#include <cib/detail/config_item.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>

namespace cib::detail {
template <auto Value>
constexpr static auto as_constant_v =
    std::integral_constant<std::remove_cvref_t<decltype(Value)>, Value>{};

template <auto... Args> struct args {};

template <typename...> struct config;

template <auto... ConfigArgs, typename... ConfigTs>
struct config<args<ConfigArgs...>, ConfigTs...> : public detail::config_item {
    stdx::tuple<ConfigTs...> configs_tuple;

    CONSTEVAL explicit config(args<ConfigArgs...>, ConfigTs const &...configs)
        : configs_tuple{configs...} {}

    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...args) const {
        return configs_tuple.apply([&](auto const &...configs_pack) {
            return stdx::tuple_cat(configs_pack.extends_tuple(
                args..., as_constant_v<ConfigArgs>...)...);
        });
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto exports_tuple(Args const &...args) const {
        return configs_tuple.apply([&](auto const &...configs_pack) {
            return stdx::tuple_cat(configs_pack.exports_tuple(
                args..., as_constant_v<ConfigArgs>...)...);
        });
    }
};

template <typename... Ts> config(Ts...) -> config<Ts...>;
} // namespace cib::detail
