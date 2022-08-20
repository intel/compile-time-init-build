#include <cib/builder_meta.hpp>
#include <cib/detail/builder_traits.hpp>

#ifndef COMPILE_TIME_INIT_BUILD_BUILT_HPP
#define COMPILE_TIME_INIT_BUILD_BUILT_HPP


namespace cib {
    /**
     * Pointer to a built service implementation.
     *
     * @tparam ServiceMeta
     *      Tag name of the service.
     *
     * @see cib::builder_meta
     */
    template<typename ServiceMeta>
    traits::interface_t<ServiceMeta> service;
}


#endif //COMPILE_TIME_INIT_BUILD_BUILT_HPP
