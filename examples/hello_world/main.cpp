#include "hello_world.hpp"

#include <cib/cib.hpp>

cib::nexus<hello_world> nexus{};

int main() {
    // services can be accessed directly from the nexus...
    nexus.service<say_message>();

    // ...or they can be accessed anywhere through cib::service
    nexus.init();
    cib::service<say_message>();

    return 0;
}