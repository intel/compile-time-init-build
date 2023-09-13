#pragma once

#include <cib/detail/config_item.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...args) const {
        return stdx::tuple_cat(Components::config.extends_tuple(args...)...);
    }

    template <typename... Args>
    [[nodiscard]] constexpr auto exports_tuple(Args const &...args) const {
        return stdx::tuple_cat(Components::config.exports_tuple(args...)...);
    }
};
} // namespace cib::detail
