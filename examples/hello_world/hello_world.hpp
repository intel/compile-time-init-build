#pragma once

#include "core.hpp"
#include "dont_panic.hpp"
#include "lazy_dog.hpp"
#include "say_hello_world.hpp"

#include <cib/cib.hpp>

struct hello_world {
    constexpr static auto config =
        cib::components<core, say_hello_world, lazy_dog, dont_panic>;
};
