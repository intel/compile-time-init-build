#pragma once

#include <nexus/detail/config_item.hpp>

#include <stdx/tuple_algorithms.hpp>

#include <boost/mp11/algorithm.hpp>

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    [[nodiscard]] constexpr auto extends_tuple() const {
        return stdx::tuple_cat(Components::config.extends_tuple()...);
    }

    [[nodiscard]] constexpr static auto get_exports() -> boost::mp11::mp_append<
        decltype(Components::config.get_exports())...> {
        return {};
    }
};
} // namespace cib::detail
