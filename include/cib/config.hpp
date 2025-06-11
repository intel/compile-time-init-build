#pragma once

#include <cib/detail/components.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/constexpr_conditional.hpp>
#include <cib/detail/exports.hpp>
#include <cib/detail/extend.hpp>
#include <cib/detail/runtime_conditional.hpp>

#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>

namespace cib {
/**
 * Container for project and component configuration declarations.
 *
 * Each component or project type must contain a static constexpr "config"
 * field that contains the cib configuration for that component or project.
 * cib::config can be used to compose multiple configuration declarations.
 *
 * @see cib::components
 * @see cib::extend
 * @see cib::exports
 * @see cib::constexpr_condition
 * @see cib::runtime_condition
 */
template <typename... Configs>
[[nodiscard]] CONSTEVAL auto config(Configs const &...configs) {
    return detail::config{configs...};
}

/**
 * Compose one or more components into a project or larger component.
 *
 * @tparam Components
 *      List of components to be added to the configuration.
 */
template <typename... Components>
constexpr static detail::components<Components...> components{};

/**
 * Declare a list of services for use in the project.
 *
 * @tparam Services
 */
template <typename... Services>
constexpr static detail::exports<Services...> exports{};

namespace detail {
template <typename T>
using maybe_funcptr_t =
    stdx::conditional_t<stdx::is_function_v<T>, std::decay_t<T>, T>;
}

/**
 * Extend a service with new functionality.
 *
 * @tparam Service
 *      Type name of the service to extend.
 *
 * @param args
 *      Value arguments to be passed to the service's builder add function.
 */
template <typename Service, typename... Args>
[[nodiscard]] CONSTEVAL auto extend(Args const &...args) {
    return detail::extend<Service, detail::maybe_funcptr_t<Args>...>{args...};
}

template <stdx::ct_string Name>
constexpr auto constexpr_condition = []<typename P>(P) {
    static_assert(std::is_default_constructible_v<P>);
    return detail::constexpr_condition<Name, P>{};
};

template <stdx::ct_string Name>
constexpr auto runtime_condition = []<typename P>(P) {
    static_assert(std::is_default_constructible_v<P>);
    return detail::runtime_condition<Name, P>{};
};
} // namespace cib
