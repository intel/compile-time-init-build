#pragma once

#include <nexus/detail/nexus_details.hpp>
#include <nexus/service.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>

#include <boost/mp11/algorithm.hpp>

#include <type_traits>

template <typename...> struct undef;

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
    template <typename T>
    constexpr static auto service_v =
        initialized<Config, T>::value
            .template build<initialized<Config, T>, nexus>();

    template <typename T> constexpr static auto service() {
        return service_v<T>();
    }

    template <stdx::ct_string Name> constexpr static auto service() {
        using Exports = decltype(Config::config.exports_tuple());
        using Idx =
            boost::mp11::mp_find_if_q<Exports, detail::matching_name<Name>>;
        if constexpr (Idx::value == stdx::tuple_size_v<Exports>) {
            STATIC_ASSERT(
                false, "Trying to invoke a service ({}) that is not exported",
                Name);
        } else {
            return service<boost::mp11::mp_at<Exports, Idx>>();
        }
    }

    static auto init() -> void {
        auto const init_interface = []<builder_meta T> {
            cib::service<T> =
                to_interface<typename T::interface_t>(service_v<T>);
            if constexpr (requires { T::name; }) {
                using R = decltype(service<T>());
                cib::invoke_service<T::name, R> = []() -> R {
                    return service<T::name>();
                };
            }
        };
        initialized_builders<Config>.apply([&]<typename... Ts>(Ts const &...) {
            (init_interface.template
             operator()<std::remove_cvref_t<typename Ts::Service>>(),
             ...);
        });
    }
};
} // namespace cib
