#pragma once

#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>

namespace cib::detail {
template <typename Pred, typename... Configs>
    requires std::is_default_constructible_v<Pred>
struct conditional : config_item {
    detail::config<Configs...> body;

    CONSTEVAL explicit conditional(Configs const &...configs)
        : body{configs...} {}

    [[nodiscard]] constexpr auto extends_tuple() const {
        if constexpr (Pred{}()) {
            return body.extends_tuple();
        } else {
            return stdx::tuple<>{};
        }
    }

    [[nodiscard]] constexpr auto exports_tuple() const {
        if constexpr (Pred{}()) {
            return body.exports_tuple();
        } else {
            return stdx::tuple<>{};
        }
    }
};
} // namespace cib::detail
