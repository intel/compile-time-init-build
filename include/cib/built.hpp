#ifndef COMPILE_TIME_INIT_BUILD_BUILT_HPP
#define COMPILE_TIME_INIT_BUILD_BUILT_HPP


#include "builder_meta.hpp"


namespace cib {
    /**
     * Pointer to a concrete built service.
     *
     * @tparam Tag Type name of the service.
     */
    template<typename Tag>
    traits::interface_t<Tag> built;
}


#endif //COMPILE_TIME_INIT_BUILD_BUILT_HPP
