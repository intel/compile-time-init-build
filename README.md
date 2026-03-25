# *cib* - Compile-time Initialization and Build

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
[![Unit Tests](https://github.com/intel/compile-time-init-build/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/intel/compile-time-init-build/actions/workflows/unit_tests.yml)

 *cib* is a C++ header-only library for building embedded firmware with reusable
components. It implements the compile-time initialization and build pattern. 
Instead of initializing components and registering callbacks at runtime, this
process is executed at compile-time using `constexpr` or `consteval` functions.

Firmware using *cib* is implemented as a collection of **components**. Each 
component provides **services** and **features** to the build. Component
**features** extend and provide new functionality to **services**.

[![emBO++ 2022 The constexpr init()/build() pattern: compose modular firmware with minimal runtime cost](https://img.youtube.com/vi/fk0ihqOXER8/0.jpg)](https://www.youtube.com/watch?v=fk0ihqOXER8)

Examples of *cib* services include:

* [Callback](include/nexus/callback.hpp) - a service used to handle simple callbacks.
* [Flow](include/flow) - a service used to compose sequences of dependent operations from multiple components.
* [Message](include/msg) - a service used match incoming data against message formats, and dispatch callbacks.

See the [full documentation](https://intel.github.io/compile-time-init-build/).

# Compiler/Standard support

C++ standard support is as follows:

- C++23: [main branch](https://github.com/intel/compile-time-init-build/tree/main) (active development)
- C++20: [cpp20 branch](https://github.com/intel/compile-time-init-build/tree/cpp20) (supported)
- C++17: [cpp17 branch](https://github.com/intel/compile-time-init-build/tree/cpp17) (end-of-life)

Compiler support:

| Branch | GCC versions | Clang versions |
| --- | --- | --- |
| [main](https://github.com/intel/compile-time-init-build/tree/main) | 12 thru 14 | 18 thru 22 |
| [cpp20](https://github.com/intel/compile-time-init-build/tree/cpp20) | 12 thru 14 | 14 thru 21 |
| [cpp17](https://github.com/intel/compile-time-init-build/tree/cpp17) | 9 thru 12 | 9 thru 15 |

# Quick start

The recommended way to use *cib* is with CMake and [CPM](https://github.com/cpm-cmake/CPM.cmake).
With this method,add the following to your `CMakeLists.txt`:

```cmake
CPMAddPackage("gh:intel/compile-time-init-build#047aab6")
target_link_libraries(your_target PRIVATE cib)
```

Where `047aab6` is the git hash (or tag, or branch) that you want to use.

Another option is to include *cib* as a 
[git submodule](https://github.blog/2016-02-01-working-with-submodules/) 
in your repo and add the *cib* directory in your `CMakeLists.txt` file:

```cmake
add_subdirectory(extern/compile-time-init-build)
target_link_libraries(your_target PRIVATE cib)
```

With either of these methods, `#include <cib/cib.hpp>` in your code to use *cib*.

## Hello, world!

Since *cib* is a library for efficiently building firmware through composition
a simple example takes a few more lines than a typical "Hello, world!"

```c++
#include <cib/cib.hpp>
#include <iostream>

struct say_message : public callback::service<>{};

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

See this example live (although from an earlier version of *cib*) at [Compiler Explorer](https://godbolt.org/z/4rsfWaPnP).

A larger and more illustrative example can be found in this repo at
[examples/nexus/hello_world](examples/nexus/hello_world).

# Sub-libraries

There are multiple sub-libraries contained within the *cib* project. See the
[library dependency
chart](https://github.com/intel/compile-time-init-build/blob/main/docs/library_deps.mmd)
for details of the dependency graph.

* [`cib`](include/cib) - an omnibus library that includes all the rest.
* [`cib_flow`](include/flow) - a flow is a sequence of dependent operations that is composed at compile time.
* [`cib_interrupt`](include/interrupt) - define and handle interrupts.
* [`cib_log`](include/log) - basics of logging; implementation-independent definitions.
* [`cib_log_binary`](include/log_binary) - a logging implementation that uses the binary [MIPI Sys-T](https://www.mipi.org/specifications/sys-t) format.
* [`cib_log_fmt`](include/log_fmt) - a logging implementation that uses [fmt](https://github.com/fmtlib/fmt).
* [`cib_lookup`](include/lookup) - select efficient lookup strategies at compile time.
* [`cib_match`](include/match) - define arbitrary match criteria for messages and callbacks.
* [`cib_msg`](include/msg) - define field and message layouts, construct messages, and set up callbacks to handle matching messages.
* [`cib_nexus`](include/nexus) - the place where services and components are composed at compile time.
* [`cib_seq`](include/seq) - a sequence is like a flow, but allows stepping and reversal.

# Dependencies

The *cib* library has the following dependencies:

* [Boost.MP11](https://github.com/boostorg/mp11)
* Intel's [C++ Bare Metal Concurrency](https://github.com/intel/cpp-baremetal-concurrency) library
* Intel's [C++ Standard Extensions](https://github.com/intel/cpp-std-extensions) library
* Intel's [C++ Bare Metal Senders & Receivers](https://github.com/intel/cpp-baremetal-senders-and-receivers) library (used in `cib_msg` only)
* [fmt](https://github.com/fmtlib/fmt) (used in `cib_log_fmt` only)

**NOTE:** *cib* uses [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) to
fetch its dependencies. When first running `cmake`, the dependencies will be
downloaded. To avoid re-downloading dependencies when reconfiguring, it's
recommended to designate a cache directory and set the `CPM_SOURCE_CACHE`
environment variable.

A few more dependencies are used only in tests:

* Intel's [Generic Register Operation Optimizer](https://github.com/intel/generic-register-operation-optimizer) library (used in `cib_interrupt` tests)
* [mph](https://github.com/boost-ext/mph) (used in `cib_lookup` benchmark tests)
* [Frozen](https://github.com/serge-sans-paille/frozen) (used in `cib_lookup` benchmark tests)

# Building

*cib* depends on Intel's [CICD Repo
Infrastructure](https://github.com/intel/cicd-repo-infrastructure) for CMake
code dealing with building and testing.

Unit tests are registered with CTest, and will build and run as part of the
built-in `all` target.

```shell
cmake -B build
cmake --build build
```

This will build and run all the unit tests. To re-run them:

```shell
ctest --test-dir build
```

## Contributing

If you'd like to contribute, please fork the repository and use a feature
branch. Pull requests are welcome.

For more details on contributing, please see [CONTRIBUTING.md](CONTRIBUTING.md)

# Links

- Repository: https://github.com/intel/compile-time-init-build/
- Issue tracker: https://github.com/intel/compile-time-init-build/issues
    - In case of sensitive bugs like security vulnerabilities, please contact
      one or more of the project maintainers directly instead of using issue 
      tracker. We value your effort to improve the security and privacy of this
      project!
    
# Licensing

The code in this project is licensed under the BSL-1.0 license. See 
[LICENSE](LICENSE) for more details.

This is a test PR
