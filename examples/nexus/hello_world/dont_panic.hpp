#pragma once

#include "core.hpp"

#include <cib/cib.hpp>

#include <iostream>

// functions declared outside the component may be used within the component
void so_long();

struct dont_panic {
    // functions can be declared in a header and defined in a object file
    static void say_it();

    // any number of extensions can be made
    constexpr static auto config = cib::config(
        cib::extend<say_message>(&say_it), cib::extend<say_message>(&so_long));
};
