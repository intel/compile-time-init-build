#include "detail/compiler.hpp"

#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP
#define COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP


namespace cib {
    template<
        typename BuilderT,
        typename InterfaceT>
    struct builder_meta {
        BuilderT builder();
        InterfaceT interface();
    };

    namespace traits {
        template<typename MetaT>
        struct builder {
            using type = decltype(std::declval<MetaT>().builder());
        };

        template<typename MetaT>
        using builder_t = typename builder<MetaT>::type;

        template<typename MetaT>
        CIB_CONSTEXPR builder_t<MetaT> builder_v = {};

        template<typename MetaT>
        struct interface {
            using type = decltype(std::declval<MetaT>().interface());
        };

        template<typename MetaT>
        using interface_t = typename interface<MetaT>::type;
    }
}


#endif //COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP
