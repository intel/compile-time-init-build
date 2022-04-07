#include "compiler.hpp"

#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_BUILDER_TRAITS_HPP
#define COMPILE_TIME_INIT_BUILD_BUILDER_TRAITS_HPP


namespace cib::traits {
    template<typename BuilderMeta>
    struct builder {
        using type = decltype(std::declval<BuilderMeta>().builder());
    };

    template<typename BuilderMeta>
    using builder_t = typename builder<BuilderMeta>::type;

    template<typename BuilderMeta>
    CIB_CONSTEXPR builder_t<BuilderMeta> builder_v = {};

    template<typename BuilderMeta>
    struct interface {
        using type = decltype(std::declval<BuilderMeta>().interface());
    };

    template<typename BuilderMeta>
    using interface_t = typename interface<BuilderMeta>::type;
}


#endif //COMPILE_TIME_INIT_BUILD_BUILDER_TRAITS_HPP
