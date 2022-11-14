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

As of the v1.0.0 release *cib*'s API is considered stable and unlikely to change.

[![emBO++ 2022 The constexpr init()/build() pattern: compose modular firmware with minimal runtime cost](https://img.youtube.com/vi/fk0ihqOXER8/0.jpg)](https://www.youtube.com/watch?v=fk0ihqOXER8)

## Sub-projects

There are multiple sub-projects contained within *cib*. Some of them are used
to implement *cib* and others extend *cib*.

* Services
    * [Flow](include/flow) - *cib* service used to compose sequences of dependent
      operations from multiple components.
    * [Callback](include/cib/callback.hpp) - *cib* service used to implement
      simple callback/listener pattern.
* Support
    * [string_constant](include/sc) - Compile-time string library with support for
      formatting similar to fmt/python format specifiers.
    * [log (wip)](include/log) - Logging library.
    * [Container](include/container) - Simple containers optimized for constexpr
      and/or free-standing applications.

## Installing / Getting started

*cib* is released as a single header file as well as the zipped github repo. 
To get started quickly, download the cib.hpp header from the 
[release area](https://github.com/intel/compile-time-init-build/releases):

```shell
wget https://github.com/intel/compile-time-init-build/releases/download/v1.0.0/cib.hpp
```

Another option is to include cib as a 
[git submodule](https://github.blog/2016-02-01-working-with-submodules/) 
in your repo and add the cib directory in your CMakeLists.txt file:

```cmake
add_subdirectory(extern/compile-time-init-build)
target_link_libraries(your_target PRIVATE cib)
```

With either of these methods, include the cib.hpp header in your code to use it.

### Hello, world!

Since *cib* is a library for efficiently building firmware through composition
a simple example takes a few more lines than a typical "Hello, world!"

```c++
#include <cib/cib.hpp>
#include <iostream>

struct say_message : public cib::callback_meta<>{};

// the 'core' component exposes the 'say_message' service for others to extend
struct core {
    constexpr static auto config = cib::exports<say_message>;
};

// the 'say_hello_world' component extends 'say_message' with its own functionality
struct say_hello_world {
    constexpr static auto config =
        cib::extend<say_message>([](){
            std::cout << "Hello, world!" << std::endl;
        });
};

// the 'hello_world' project composes 'core' and 'say_hello_world'
struct hello_world {
    constexpr static auto config =
        cib::components<core, say_hello_world>;
};

// the nexus instantiates the project
cib::nexus<hello_world> nexus{};

int main() {
    // the fully extended and built services are ready to be used
    nexus.service<say_message>();
    return 0;
}
```

Try out this example live at [Compiler Explorer](https://godbolt.org/z/4rsfWaPnP).

A larger and more illustrative example can be found in this repo at
[examples/hello_world](examples/hello_world).

For more details on how to use *cib*, see the [User Guide](USER_GUIDE.md).

### Building

*cib* is built with CMake. The single header is built with the 
`release_header` target:

```shell
git clone https://github.com/intel/compile-time-init-build.git
cmake -B build
cmake --build build -t release_header
ls build/include/cib/ | grep cib.hpp
```

This combines all the *cib* header files in the `include` tree by recursively
including the `#include` directives and ignoring all other macros.

**NOTE:** *cib* uses [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) to
fetch its dependencies. When first running `cmake`, the dependencies will be
downloaded. To avoid re-downloading dependencies when reconfiguring cmake, it's
recommended to designate a cache directory and set the `CPM_SOURCE_CACHE`
environment variable.

Unit tests are registered with CTest, and will build and run as part of the
built-in `all` target.

```shell
cmake -B build
cmake --build build
```

This will build and run all the unit tests with Catch2 and GTest. To re-run them:

```shell
ctest --test-dir build
```

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
[LICENSE](LICENSE) for more details.
