# *cib* User Guide

## Overview

*cib* allows for firmware to be efficiently composed of self-contained
*components*. A *component* is composed of zero or more *features* and
*services*.

> *cib* introduces new terminology with concrete definitions. Whenever a *cib*
concept is written, it will be *italic*. Whenever a C++ type or function is
written, it will be formatted as `code`.

### Services

> A *service* is something that can be *extended* with new functionality. 
 
For example, a serial port that can receive messages has a driver for 
interfacing with the hardware. When data is received over the serial port, the
driver needs to direct the data to the appropriate *feature*. How does the 
driver know what *features* to send the data to? 

If the serial port driver is implemented as a *service* with *cib*, then 
*features* will `extend` the serial port *service* with their own 
functionality.

```c++
/// Invoked for each byte of data received on the serial port
struct serial_port_rx : public cib::callback_meta<0, std::uint8_t>{};
```

In *cib*, *features* have source-code dependencies on *services*. This follows
the [Dependency Inversion Principle](https://en.wikipedia.org/wiki/Dependency_inversion_principle):

1. High-level modules should not import anything from low-level modules. Both 
   should depend on abstractions (e.g., interfaces).
2. Abstractions should not depend on details. Details (concrete implementations)
   should depend on abstractions.

*Features* maybe change from one project to the next. The selection of which
*features* are in a project will change as well. They have the most change and
are the least stable.

*Services* on the other hand are stable. They provide generic functionality
that can be reused over and over again.

During firmware startup, there will be hardware registers that need to be 
initialized before they can be used. The `runtime_init` and `main_loop`  
*services* are generic enough to be used in many types of firmware applications.

```c++
/// Invoked once on startup before interrupts are enabled
struct runtime_init : public cib::callback_meta<>{};

/// Invoked each iteration through the main loop
struct main_loop : public cib::callback_meta<>{};
```

*Components* use `cib::exports` in their configuration to *export* services to 
features. All *services* must be exported for them to be extended.

```c++
struct board_component {
    constexpr static auto config =
         cib::exports<serial_port_rx, runtime_init, main_loop>; 
};
```

### Features 

> *Features* are the code that performs the work we are actually interested in.

In application development this would be called the "business logic." In 
systems programming this is the code that gets the job done.

With *cib*, *features* `extend` services with their functionality.

```c++
/// Echo serial port rx data back to tx
struct echo_component {
    constexpr static auto echo_feature =
       [](std::uint8_t data){
           serial_port.transmit(data);
       };
    
    constexpr static auto config =
        cib::extend<serial_port_rx>(echo_feature);
};
```

### Project

> A project is a collection of *components*. 

It is the embedded application that is being developed. The implementation of
the application is entirely contained within the components it comprises.

Only a small amount of startup and glue-code is necessary outside *cib*
*components*.

### Nexus

> The `cib::nexus` combines all the *services* and *features* within a project.
It performs the compile-time initialization and build process across all
components.

The [definition](https://www.google.com/search?q=define+nexus) of *nexus* fits `cib::nexus` well:

1. a connection or series of connections linking two or more things.

   "the nexus between industry and political power“

2. connected group or series.

   "a nexus of ideas“

3. the central and most important point or place.

   "the nexus of all this activity was the disco"

The `cib::nexus` implements the heart of *cib*. Once a *cib* configuration has
been created, using the `cib::nexus` is easy:

```c++
cib::nexus<hello_world> nexus{};

int main() {
    nexus.init();
    nexus.service<runtime_init>();

    while (true) {
        nexus.service<main_loop>();
    }
}
```

Services can be accessed with the `service<...>` template variable on a 
`cib::nexus` instance. Because the `runtime_init` and `main_loop` services
extend `cib::callback_meta`, their *service implementation* is a simple 
function pointer.