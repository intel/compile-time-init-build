# Hello, world!

Since *cib* is a library for efficiently building firmware through composition
a simple example takes a few more lines than a typical "Hello, world!"

This example shows how a core *service*, `say_message`, can be extended by 
multiple independent components using *cib*.

#### [core.cpp](core.cpp)
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
#### [say_hello_world.cpp](say_hello_world.cpp)
The `hello_world` component **extends** the `say_message` **service** with new
contained in a lambda.
```c++
#include <iostream>
#include <cib/cib.hpp>

struct say_hello_world {
    constexpr static auto config =
        cib::config(
            cib::extend<say_message>([](){
                std::cout << "Hello, world!" << std::endl;
            })        
        );
};
```
#### [lazy_dog.cpp](lazy_dog.cpp)
Another component, `lazy_dog` is also extending the `say_message` **service**.
This time it is using a function pointer instead of a lambda. The function 
definition of `talk_about_the_dog` could also be placed in a `lazy_dog.cpp` 
file if desired.

Note the use of the `&` operator to take the function pointer of 
`talk_about_the_dog`.

```c++
#include <iostream>
#include <cib/cib.hpp>

struct lazy_dog {
    static void talk_about_the_dog() {
        std::cout << "The quick brown fox jumps over the lazy dog." << std::endl;
    }
    
    constexpr static auto config =
        cib::config(
            cib::extend<say_message>(&talk_about_the_dog)        
        );
};
```
#### [dont_panic.hpp](dont_panic.hpp)
The `dont_panic` component illustrates how functions defined in object files
may still be used to extend *services*. 

```c++
#include <iostream>
#include <cib/cib.hpp>

// functions declared outside the component may be used within the component
void so_long();

struct dont_panic {
    // functions can be declared in a header and defined in a object file
    static void say_it();
    
    // any number of extensions can be made
    constexpr static auto config =
        cib::config(
            cib::extend<say_message>(&say_it),
            cib::extend<say_message>(&so_long)
        );
};
```
#### [hello_world.cpp](hello_world.cpp)
All the components are brought together in the project configuration, `my_project`.
```c++
#include "core.hpp"
#include "say_hello_world.hpp"
#include "lazy_dog.hpp"

struct hello_world {
    constexpr static auto config =
        cib::components<core, say_hello_world, lazy_dog>;
};
```
#### [main.cpp](main.cpp)
The `cib::nexus` brings all the **services** and **features** together. This is
where the compile-time initialization and build process actually occurs.
```c++
#include "hello_world.hpp"

cib::nexus<hello_world> nexus{};

int main() {
    // services can be accessed directly from the nexus...
    nexus.service<say_message>();
    
    // ...or they can be accessed anywhere through cib::service
    nexus.init();
    cib::service<say_message>();
    return 0;
}
```

#### Build
Use cmake to build the hello_world example:
```shell
cmake -B build
cmake --build build
```

#### Execution
All of the initialization and registration occurs at compile-time, but the
new functionality is still executed at run-time:
```
shell> ./build/hello_world
So long and thanks for all the fish.
Don't Panic.
The quick brown fox jumps over the lazy dog.
Hello, world!
So long and thanks for all the fish.
Don't Panic.
The quick brown fox jumps over the lazy dog.
Hello, world!
```

### User guide

For more details on how to use *cib*, see the [User Guide](../../USER_GUIDE.md).