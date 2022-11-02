#include <cib/builder_meta.hpp>
#include <cib/detail/builder_traits.hpp>
#include <cib/detail/compiler.hpp>
#include <cib/detail/components.hpp>
#include <cib/detail/conditional.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/exports.hpp>
#include <cib/detail/extend.hpp>
#include <cib/detail/meta.hpp>

#include <type_traits>

#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_HPP

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
[[nodiscard]] CIB_CONSTEVAL auto config(Configs const &...configs) {
    return detail::config{args<>, configs...};
}

template <auto... Args, typename... Configs>
[[nodiscard]] CIB_CONSTEVAL auto config(detail::args<Args...> config_args,
                                        Configs const &...configs) {
    return detail::config{config_args, configs...};
}

/**
 * Compose one or more components into a project or larger component.
 *
 * @tparam Args
 *      Template instantiation of cib::args filled with compile-time
 *      configurations values.
 *
 * @tparam Components
 *      List of components to be added to the configuration.
 *
 * @see cib::args
 */
template <typename Args, typename... Components>
CIB_CONSTEXPR static detail::components<Args, Components...> components{};

/**
 * Declare a list of services for use in the project.
 *
 * @tparam Services
 */
template <typename... Services>
CIB_CONSTEXPR static detail::exports<Services...> exports{};

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
[[nodiscard]] CIB_CONSTEVAL auto extend(Args const &...args) {
    return detail::extend<Service, Args...>{args...};
}

/**
 * Reference an argument by type.
 *
 * @tparam ArgType
 *      The type of argument to reference from the args list.
 */
template <typename ArgType> CIB_CONSTEXPR static detail::arg<ArgType> arg{};

/**
 * Include configs based on predicate.
 *
 * If predicate evaluates to true, then the configs will be added to the
 * configuration. Otherwise the configs contained in this conditional
 * will not be added.
 */
template <typename Predicate, typename... Configs>
[[nodiscard]] CIB_CONSTEVAL auto conditional(Predicate const &predicate,
                                             Configs const &...configs) {
    return detail::conditional{predicate, configs...};
}
} // namespace cib

#endif // COMPILE_TIME_INIT_BUILD_CONFIG_HPP
