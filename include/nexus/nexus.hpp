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
    template <typename Tag>
    constexpr static auto service = [] {
        using init_t = initialized<Config, Tag>;
        return init_t::value.template build<init_t>();
    }();

    static void init() {
        auto const service = []<typename T> {
            auto &service_impl = nexus::service<T>;
            using from_t = std::remove_cvref_t<decltype(service_impl)>;
            using to_t = std::remove_cvref_t<decltype(cib::service<T>)>;

            if constexpr (std::is_convertible_v<from_t, to_t>) {
                cib::service<T> = service_impl;
            } else {
                if constexpr (std::is_pointer_v<from_t>) {
                    cib::service<T> = service_impl;
                } else {
                    cib::service<T> = &service_impl;
                }
            }
        };
        initialized_builders<Config>.apply([&]<typename... Ts>(Ts const &...) {
            (service.template
             operator()<std::remove_cvref_t<typename Ts::Service>>(),
             ...);
        });
    }
};
} // namespace cib
