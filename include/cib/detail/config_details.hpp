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

template <auto... Args> struct args {
    constexpr static auto value = stdx::make_tuple(as_constant_v<Args>...);
};

template <typename ConfigArgs, typename... ConfigTs>
struct config : public detail::config_item {
    stdx::tuple<ConfigTs...> configs_tuple;

    CONSTEVAL explicit config(ConfigArgs, ConfigTs const &...configs)
        : configs_tuple{configs...} {}

    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...args) const {
        return ConfigArgs::value.apply([&](auto const &...config_args) {
            return configs_tuple.apply([&](auto const &...configs_pack) {
                return stdx::tuple_cat(
                    configs_pack.extends_tuple(args..., config_args...)...);
            });
        });
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto exports_tuple(Args const &...args) const {
        return ConfigArgs::value.apply([&](auto const &...config_args) {
            return configs_tuple.apply([&](auto const &...configs_pack) {
                return stdx::tuple_cat(
                    configs_pack.exports_tuple(args..., config_args...)...);
            });
        });
    }
};
} // namespace cib::detail
