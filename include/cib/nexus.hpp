#pragma once

#include <cib/built.hpp>
#include <cib/detail/nexus_details.hpp>

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
  private:
    using this_t = nexus<Config>;

// Workaround unfortunate bug in clang where it can't deduce "auto" sometimes
#define CIB_BUILD_SERVICE                                                      \
    initialized<Config, Tag>::value.template build<initialized<Config, Tag>>()

  public:
    template <typename Tag>
    constexpr static decltype(CIB_BUILD_SERVICE) service = CIB_BUILD_SERVICE;
#undef CIB_BUILD_SERVICE

    static void init() {
        auto const service = []<typename T> {
            using from_t = std::remove_cvref_t<decltype(this_t::service<T>)>;
            using to_t = std::remove_cvref_t<decltype(cib::service<T>)>;

            auto &service_impl = this_t::service<T>;
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
