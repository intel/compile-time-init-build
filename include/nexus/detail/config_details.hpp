#pragma once

#include <nexus/detail/config_item.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>

namespace cib::detail {
template <auto Value>
constexpr static auto as_constant_v =
    std::integral_constant<std::remove_cvref_t<decltype(Value)>, Value>{};

template <typename... ConfigTs> struct config : public detail::config_item {
    stdx::tuple<ConfigTs...> configs_tuple;

    CONSTEVAL explicit config(ConfigTs const &...configs)
        : configs_tuple{configs...} {}

    [[nodiscard]] constexpr auto extends_tuple() const {
        return configs_tuple.apply([&](auto const &...configs_pack) {
            return stdx::tuple_cat(configs_pack.extends_tuple()...);
        });
    }

    [[nodiscard]] constexpr auto exports_tuple() const {
        return configs_tuple.apply([&](auto const &...configs_pack) {
            return stdx::tuple_cat(configs_pack.exports_tuple()...);
        });
    }
};

template <typename... Ts> config(Ts...) -> config<Ts...>;
} // namespace cib::detail
