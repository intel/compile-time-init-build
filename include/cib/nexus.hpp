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
        initialized_builders<Config>.for_each([](auto b) {
            using service_tag = typename decltype(b)::Service;
            using clean_service_tag =
                std::remove_cv_t<std::remove_reference_t<service_tag>>;

            auto &service_impl = this_t::service<clean_service_tag>;
            using service_impl_type =
                std::remove_reference_t<decltype(service_impl)>;

            if constexpr (std::is_pointer_v<service_impl_type>) {
                cib::service<clean_service_tag> = service_impl;

            } else {
                cib::service<clean_service_tag> = &service_impl;
            }
        });
    }
};
} // namespace cib
