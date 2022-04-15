#ifndef CIB_HELLO_WORLD_HPP
#define CIB_HELLO_WORLD_HPP


#include "core.hpp"
#include "say_hello_world.hpp"
#include "lazy_dog.hpp"
#include "dont_panic.hpp"

#include <cib/cib.hpp>


struct hello_world {
    constexpr static auto config =
        cib::components<core, say_hello_world, lazy_dog, dont_panic>;
};


#endif //CIB_HELLO_WORLD_HPP
