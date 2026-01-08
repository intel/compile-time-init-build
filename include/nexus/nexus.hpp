#pragma once

#include <nexus/detail/nexus_details.hpp>
#include <nexus/service.hpp>

#include <type_traits>

namespace cib {
/**
 * Combines all components in a single location so their features can
 * extend services.
 *
 * @tparam Config
 *      Project configuration class that contains a single constexpr static
 *      "config" field describing the cib::config
 *
 * @see cib::config
 */

template <typename Config> struct nexus {
    template <builder_meta T>
    constexpr static auto service = [] {
        using init_t = initialized<Config, T>;
        return init_t::value.template build<init_t, nexus>();
    }();

    static void init() {
        auto const init_interface = []<builder_meta T> {
            cib::service<T> = to_interface<typename T::interface_t>(service<T>);
        };
        initialized_builders<Config>.apply([&]<typename... Ts>(Ts const &...) {
            (init_interface.template
             operator()<std::remove_cvref_t<typename Ts::Service>>(),
             ...);
        });
    }
};
} // namespace cib
