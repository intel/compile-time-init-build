#pragma once

#include <cib/detail/config_item.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    [[nodiscard]] constexpr auto extends_tuple() const {
        return stdx::tuple_cat(Components::config.extends_tuple()...);
    }

    [[nodiscard]] constexpr auto exports_tuple() const {
        return stdx::tuple_cat(Components::config.exports_tuple()...);
    }
};
} // namespace cib::detail
