#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/tuple.hpp>

namespace cib::detail {
template <typename Pred, typename... Configs>
    requires std::is_default_constructible_v<Pred>
struct conditional : config_item {
    detail::config<detail::args<>, Configs...> body;

    CIB_CONSTEVAL explicit conditional(Configs const &...configs)
        : body{{}, configs...} {}

    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...) const {
        if constexpr (Pred{}(Args{}...)) {
            return body.extends_tuple(Args{}...);
        } else {
            return cib::tuple<>{};
        }
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto exports_tuple(Args const &...) const {
        if constexpr (Pred{}(Args{}...)) {
            return body.exports_tuple(Args{}...);
        } else {
            return cib::tuple<>{};
        }
    }
};
} // namespace cib::detail
