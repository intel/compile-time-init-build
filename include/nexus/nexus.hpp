#pragma once

#include <nexus/detail/nexus_details.hpp>
#include <nexus/service.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/function_traits.hpp>

#include <boost/mp11/algorithm.hpp>

#include <type_traits>
#include <utility>

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

    template <typename T, typename... Args>
    constexpr static auto service(Args &&...args) {
        return service_v<T>(std::forward<Args>(args)...);
    }

    template <stdx::ct_string Name, typename... Args>
    constexpr static auto service(Args &&...args) {
        using Exports = decltype(Config::config.get_exports());
        using Idx =
            boost::mp11::mp_find_if_q<Exports, stdx::matching_name_q<Name>>;
        if constexpr (Idx::value == boost::mp11::mp_size<Exports>::value) {
            STATIC_ASSERT(
                false, "Trying to invoke a service ({}) that is not exported",
                Name);
        } else {
            return service<boost::mp11::mp_at<Exports, Idx>>(
                std::forward<Args>(args)...);
        }
    }

    static auto init() -> void {
        auto const init_interface = []<builder_meta T> {
            using F = typename T::interface_t;
            cib::service<T> = to_interface<F>(service_v<T>);
            if constexpr (requires { stdx::name_of_v<T>; }) {
                using R = stdx::return_t<F>;
                []<typename... Args>(boost::mp11::mp_list<Args...>) {
                    cib::invoke_service<stdx::name_of_v<T>, R, Args...> =
                        [](Args... args) -> R {
                        return service<stdx::name_of_v<T>>(
                            std::forward<std::remove_cvref_t<Args>>(args)...);
                    };
                }(stdx::args_t<F, boost::mp11::mp_list>{});
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
