#pragma once

#include <cib/cib.hpp>

#include <iostream>

struct say_hello_world {
    constexpr static auto config = cib::config(cib::extend<say_message>(
        []() { std::cout << "Hello, world!" << std::endl; }));
};
