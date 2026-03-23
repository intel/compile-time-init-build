#pragma once

#include <nexus/conditionals.hpp>
#include <nexus/detail/components.hpp>
#include <nexus/detail/config_details.hpp>
#include <nexus/detail/exports.hpp>
#include <nexus/extend.hpp>

#include <stdx/compiler.hpp>

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
} // namespace cib
