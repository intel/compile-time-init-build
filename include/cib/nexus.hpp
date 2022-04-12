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
        template<typename T>
        using raw_service_t = decltype(initialized<Config, T>::value.template build<initialized<Config, T>>());

        template<typename T>
        CIB_CONSTINIT static inline raw_service_t<T> raw_service =
            initialized<Config, T>::value.template build<initialized<Config, T>>();

        template<typename Tag>
        using service_name_to_raw_type_t =
            typename std::remove_const_t<std::remove_reference_t<decltype(cib::detail::find<Tag, cib::ServiceTagMetaFunc>(cib::initialized_builders_v<Config>))>>;

        template<typename T>
        using service_t =
            decltype(initialized<Config, service_name_to_raw_type_t<T>>::value.template build<initialized<Config, service_name_to_raw_type_t<T>>>());

    public:
        template<typename T>
        CIB_CONSTINIT static inline service_t<T> service =
            initialized<Config, service_name_to_raw_type_t<T>>::value.template build<initialized<Config, service_name_to_raw_type_t<T>>>();

        static void init() {
            detail::for_each(initialized_builders_v<Config>, [](auto b){
                // Tag/CleanTag is the type name of the builder_meta in the ordered_set
                using Tag = typename decltype(b)::Service;
                using CleanTag = std::remove_cv_t<std::remove_reference_t<Tag>>;

                using CleanTypename = std::remove_cv_t<std::remove_reference_t<decltype(b)>>;

                // the built implementation is stored in Build<>::value
                auto & builtValue = raw_service<CleanTypename>;
                using BuiltType = std::remove_reference_t<decltype(builtValue)>;

                // if the built type is a pointer, then it is a function pointer and we should return its value
                // directly to the 'built<>' abstract interface variable.
                if constexpr(std::is_pointer_v<BuiltType>) {
                    cib::service<CleanTag> = builtValue;

                // if the built type is not a pointer, then it is a class and the 'built<>' variable is a pointer to
                // the base class. we will need to get a pointer to the builtValue in order to initialize 'built<>'.
                } else {
                    cib::service<CleanTag> = &builtValue;
                }
            });
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_HPP
