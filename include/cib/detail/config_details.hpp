#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/meta.hpp>
#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>

#include <type_traits>

namespace cib::detail {
template <auto Value>
constexpr static auto as_constant_v = std::integral_constant<
    std::remove_cv_t<std::remove_reference_t<decltype(Value)>>, Value>{};

template <auto... Args> struct args {
    static constexpr auto value = cib::make_tuple(as_constant_v<Args>...);
};

template <typename ConfigArgs, typename... ConfigTs>
struct config : public detail::config_item {
    cib::tuple<ConfigTs...> configs_tuple;

    CIB_CONSTEVAL explicit config(ConfigArgs, ConfigTs const &...configs)
        : configs_tuple{configs...} {}

    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...args) const {
        return ConfigArgs::value.apply([&](auto const &...config_args) {
            return configs_tuple.apply([&](auto const &...configs_pack) {
                return cib::tuple_cat(
                    configs_pack.extends_tuple(args..., config_args...)...);
            });
        });
    }
};
} // namespace cib::detail
