#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/tuple.hpp>

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    template <typename... Args>
    [[nodiscard]] CIB_CONSTEXPR auto extends_tuple(Args const &...args) const {
        return cib::tuple_cat(Components::config.extends_tuple(args...)...);
    }
};
} // namespace cib::detail
