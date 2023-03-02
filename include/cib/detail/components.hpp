#pragma once

#include <cib/detail/config_item.hpp>
#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...args) const {
        return cib::tuple_cat(Components::config.extends_tuple(args...)...);
    }
};
} // namespace cib::detail
