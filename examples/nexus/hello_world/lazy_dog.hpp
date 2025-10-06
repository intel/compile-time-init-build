#pragma once

#include <cib/cib.hpp>

#include <iostream>

struct lazy_dog {
    static void talk_about_the_dog() {
        std::cout << "The quick brown fox jumps over the lazy dog."
                  << std::endl;
    }

    constexpr static auto config =
        cib::config(cib::extend<say_message>(&talk_about_the_dog));
};
