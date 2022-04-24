#include "detail/nexus_details.hpp"
#include "detail/compiler.hpp"
#include "detail/meta.hpp"
#include "built.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_NEXUS_HPP
#define COMPILE_TIME_INIT_BUILD_NEXUS_HPP


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
    template<typename Config>
    struct nexus {
    private:
        using this_t = nexus<Config>;

        template<typename Tag>
        using service_name_to_raw_type_t =
            typename std::remove_const_t<std::remove_reference_t<decltype(cib::detail::find<Tag, cib::ServiceTagMetaFunc>(cib::initialized_builders_v<Config>))>>;

        // Workaround unfortunate bug in clang where it can't deduce "auto" sometimes
        #define CIB_BUILD_SERVICE initialized<Config, service_name_to_raw_type_t<T>>::value.template build<initialized<Config, service_name_to_raw_type_t<T>>>()

    public:
        template<typename T>
        constexpr static decltype(CIB_BUILD_SERVICE) service = CIB_BUILD_SERVICE;
        #undef CIB_BUILD_SERVICE

        static void init() {
            detail::for_each(initialized_builders_v<Config>, [](auto b){
                using service_tag = typename decltype(b)::Service;
                using clean_service_tag = std::remove_cv_t<std::remove_reference_t<service_tag>>;

                auto & service_impl = this_t::service<clean_service_tag>;
                using service_impl_type = std::remove_reference_t<decltype(service_impl)>;

                if constexpr(std::is_pointer_v<service_impl_type>) {
                    cib::service<clean_service_tag> = service_impl;

                } else {
                    cib::service<clean_service_tag> = &service_impl;
                }
            });
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_HPP
