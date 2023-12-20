#pragma once

#include <cib/builder_meta.hpp>
#include <cib/detail/components.hpp>
#include <cib/detail/conditional.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/exports.hpp>
#include <cib/detail/extend.hpp>

#include <stdx/compiler.hpp>

namespace cib {
/**
 * List of arguments to configure compile-time initialization of components.
 *
 * @see cib::conditional
 */
template <auto... Args> constexpr static detail::args<Args...> args{};

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
 * @see cib::conditional
 */
template <typename... Configs>
[[nodiscard]] CONSTEVAL auto config(Configs const &...configs) {
    return detail::config{args<>, configs...};
}

template <auto... Args, typename... Configs>
[[nodiscard]] CONSTEVAL auto config(detail::args<Args...> config_args,
                                    Configs const &...configs) {
    return detail::config{config_args, configs...};
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

/**
 * Extend a service with new functionality.
 *
 * @tparam Service
 *      Type name of the service to extend.
 *
 * @tparam ServiceTemplateArgs
 *      Template arguments to be passed to the service's
 *      builder add function.
 *
 * @param args
 *      Value arguments to be passed to the service's builder add function.
 */
template <typename Service, typename... Args>
[[nodiscard]] CONSTEVAL auto extend(Args const &...args) {
    return detail::extend<Service, Args...>{args...};
}

/**
 * Include configs based on predicate.
 *
 * If predicate evaluates to true, then the configs will be added to the
 * configuration. Otherwise the configs contained in this conditional
 * will not be added.
 */
template <typename Predicate, typename... Configs>
    requires std::is_default_constructible_v<Predicate>
[[nodiscard]] CONSTEVAL auto conditional(Predicate const &,
                                         Configs const &...configs) {
    return detail::conditional<Predicate, Configs...>{configs...};
}
} // namespace cib
