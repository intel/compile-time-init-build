# *cib* - Compile-time Initialization and Build

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
[![Unit Tests](https://github.com/intel/compile-time-init-build/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/intel/compile-time-init-build/actions/workflows/unit_tests.yml)

 *cib* is a C++ header-only library for building embedded firmware with reusable
components. It implements the compile-time initialization and build pattern. 
Instead of initializing components and registering callbacks at runtime, this
process is executed at compile-time using constexpr or consteval functions.

Firmware using *cib* is implemented as a collection of **components**. Each 
component provides **services** and **features** to the build. Component
**features** extend and provide new functionality to **services**.

*cib* is currently in a prerelease state and its API is subject to change.

## Installing / Getting started

*cib* is released as a single header file as well as the zipped github repo. 
To get started quickly, download the cib.hpp header from the 
[release area](https://github.com/intel/compile-time-init-build/releases):

```shell
wget https://github.com/intel/compile-time-init-build/releases/download/v0.1.0/cib.hpp
```

Another option is to include cib as a 
[git submodule](https://github.blog/2016-02-01-working-with-submodules/) 
in your repo and add the cib directory in your CMakeLists.txt file:

```cmake
add_subdirectory(lib/compile-time-init-build)
target_link_libraries(your_target PRIVATE cib)
```

With either of these methods, include the cib.hpp header in your code to use it.

### Hello, world!

Since *cib* is a library for efficiently building firmware through composition
a simple example takes a few more lines than a typical "Hello, world!"

#### core.hpp
The `core` component of this example **exports** the `say_message` **service**. Pay close
attention to the `#include` directives in each file.
```c++
#include <cib/cib.hpp>

struct say_message : public cib::callback_meta<>{};

struct core {
    constexpr static auto config =
        cib::config(cib::exports<say_message>);
};
```
#### hello_world.hpp
The `hello_world` component **extends** the `say_message` **service** with new
contained in a lambda.
```c++
#include <iostream>
#include <cib/cib.hpp>

struct hello_world {
    constexpr static auto config =
        cib::config(
            cib::extend<say_message>([](){
                std::cout << "Hello, world!" << std::endl;
            })        
        );
};
```
#### lazy_dog.hpp
Another component, `lazy_dog` is also extending the `say_message` **service**.
This time it is using a function pointer instead of a lambda. The function 
definition of `talk_about_the_dog` could also be placed in a `lazy_dog.cpp` 
file if desired.
```c++
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
```
#### my_project.hpp
All the components are brought together in the project configuration, `my_project`.
```c++
#include "core.hpp"
#include "hello_world.hpp"
#include "lazy_dog.hpp"

struct my_project {
    constexpr static auto config =
        cib::components<core, hello_world, lazy_dog>;
};
```
#### main.cpp
The `cib::nexus` brings all the **services** and **features** together. This is
where the compile-time initialization and build process actually occurs.
```c++
#include "my_project.hpp"

cib::nexus<my_project> nexus{};

int main() {
    // services can be accessed directly from the nexus...
    nexus.service<say_message>();
    
    // ...or they can be accessed anywhere through cib::service
    nexus.init();
    cib::service<say_message>();
}
```
#### Execution
All of the initialization and registration occurs at compile-time, but the
new functionality is still executed at run-time:
```
shell> ./my_project
Hello, world!
The quick brown fox jumps over the lazy dog.
Hello, world!
The quick brown fox jumps over the lazy dog.
```

### Building

*cib* is built with CMake. The single header is built with the 
`release_header` target:

```shell
cmake -B build
cmake --build build -t release_header
ls build/include/cib/ | grep cib.hpp
```

This combines all the *cib* header files in the `include` tree by recursively
including the `#include` directives and ignoring all other macros.

Unit tests are registered with CTest:

```shell
cmake -B build
cmake --build build -t tests
ctest
```

This will build and run all the unit tests with CMake and Catch2.

## Features

* Compose modular firmware systems with high-level abstractions
* Perform registration of components at compile-time
  * 🏎 Optimize runtime-performance and memory usage
  * 🦺 Catch undefined behavior during initialization

## Contributing

If you'd like to contribute, please fork the repository and use a feature
branch. Pull requests are warmly welcome.

For more details on contributing, please see [CONTRIBUTING.md](CONTRIBUTING.md)

## Links

- Repository: https://github.com/intel/compile-time-init-build/
- Issue tracker: https://github.com/intel/compile-time-init-build/issues
    - In case of sensitive bugs like security vulnerabilities, please contact
      one or more of the project maintainers directly instead of using issue 
      tracker. We value your effort to improve the security and privacy of this
      project!
    
## Licensing

The code in this project is licensed under the BSL-1.0 license. See 
[LICENSE.md](LICENSE.md) for more details.
