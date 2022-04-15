#ifndef CIB_LAZY_DOG_HPP
#define CIB_LAZY_DOG_HPP


#include <iostream>
#include <cib/cib.hpp>


struct lazy_dog {
    static void talk_about_the_dog() {
        std::cout << "The quick brown fox jumps over the lazy dog." << std::endl;
    }

    constexpr static auto config =
        cib::config(
            cib::extend<say_message>(talk_about_the_dog)
        );
};


#endif //CIB_LAZY_DOG_HPP
