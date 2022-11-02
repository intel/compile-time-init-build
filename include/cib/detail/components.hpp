#include <cib/detail/compiler.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/meta.hpp>
#include <cib/detail/type_list.hpp>
#include <cib/tuple.hpp>

#include <type_traits>

#ifndef COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
#define COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP

namespace cib::detail {
template <typename... Components>
struct components : public detail::config_item {
    template <typename... Args>
    [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(Args const &...args) const {
        return cib::tuple_cat(Components::config.extends_tuple(args...)...);
    }
};
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
