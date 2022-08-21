# Flow

A flow is a series of actions to be executed in order based on their dependencies. The flow library was originally
designed to be used for power flows, moving from one power state to another. It can also be used as a generic extension
point to register callbacks at compile-time. Flow implements the constexpr init()/build() pattern.

## Usage 

Flow extension points are declared by extending the `flow::service`.

```c++
struct MorningRoutine : public flow::service<decltype("MorningRoutine"_sc)> {}; 
```

The builder is used during the constexpr init() phase to define and extend the flow. Actions to be added to the flow are
declared and defined as constexpr constants. All actions added to a flow will be executed.

```c++
namespace food { 
    constexpr static auto MAKE_COFFEE = flow::action("MAKE_COFFEE"_sc, [] { 
        coffee_maker.start(ground_coffee.take(100_grams), water.take(16_ounces)); 
    }); 
    
    constexpr static auto DRINK_COFFEE = flow::action("DRINK_COFFEE"_sc, [] { 
        ...
    }); 
} 
```

Actions are added to the flow inside a component's `cib::config`.

```c++
struct morning { 
    constexpr auto config = cib::config(
        cib::exports<MorningRoutine>,
        cib::extend<MorningRoutine>(
            WAKE_UP >>
            selfcare::SHOWER >>
            selfcare::GET_DRESSED >>
            food::MAKE_COFFEE >>
            food::DRINK_COFFEE
        )
    );
} 
```

The >> operator is used to create a dependency between two actions. For example `WAKE_UP >> SHOWER` means you need to
wake up first before you can take a shower. The flow library will order actions in a flow to respect these dependencies.
The actions will be executed in an order that respects all given dependencies.

If we only use the `morning` component in our project, the `MorningRoutine` flow graph would look like the following:

```
    ┌─MorningRoutine──────────┐
    │                         │
    │  WAKE_UP                │
    │     │                   │
    │     ▼                   │
    │  selfcare::SHOWER       │
    │     │                   │
    │     ▼                   │
    │  selfcare::GET_DRESSED  │
    │     │                   │
    │     ▼                   │
    │  food::MAKE_COFFEE      │
    │     │                   │
    │     ▼                   │
    │  food::DRINK_COFFEE     │
    │                         │
    └─────────────────────────┘
```

The power of `flow` services comes when more than one component adds actions to the flow. Flows can be extended by 
inserting additional actions with new dependencies.

```c++
struct childcare { 
    constexpr static auto PACK_SCHOOL_LUNCHES = flow::action("PACK_SCHOOL_LUNCHES"_sc, [] { 
        ...
    }); 
    
    constexpr static auto SEND_KIDS_TO_SCHOOL = flow::action("SEND_KIDS_TO_SCHOOL"_sc, [] { 
        ...
    }); 
    
    constexpr auto config = cib::config(
        cib::extend<MorningRoutine>(
            food::MAKE_COFFEE >>          // this step exists in the core MorningRoutine flow 
            PACK_SCHOOL_LUNCHES >>        // new
            food::DRINK_COFFEE >>         // existing 
            food::MAKE_BREAKFAST >>       // new 
            food::EAT_BREAKFAST >>        // new
            SEND_KIDS_TO_SCHOOL           // new
        )
    );
} 
```

The new steps are inserted into the existing `flow`'s dependency graph:

```
    ┌─MorningRoutine─────────────┐
    │                            │
    │  WAKE_UP                   │
    │     │                      │
    │     ▼                      │
    │  selfcare::SHOWER          │
    │     │                      │
    │     ▼                      │
    │  selfcare::GET_DRESSED     │
    │     │                      │
    │     ▼                      │
    │  food::MAKE_COFFEE         │
    │     │  │                   │
    │     │  ▼                   │
    │     │ PACK_SCHOOL_LUNCHES  │
    │     │  │                   │
    │     ▼  ▼                   │
    │  food::DRINK_COFFEE        │
    │     │                      │
    │     ▼                      │
    │  food::MAKE_BREAKFAST      │
    │     │                      │
    │     ▼                      │
    │  food::EAT_BREAKFAST       │
    │     │                      │
    │     ▼                      │
    │  SEND_KIDS_TO_SCHOOL       │
    │                            │
    └────────────────────────────┘
```

Multiple independent components can add actions to the same `flow`. This is the power of `flow` services, they can be
extended by multiple independent components to create new functionality.

```c++
namespace exercise { 
    constexpr static auto RIDE_STATIONARY_BIKE = flow::action("RIDE_STATIONARY_BIKE"_sc, [] { 
        ...
    }); 
    
    constexpr auto config = cib::config(
        cib::extend<MorningRoutine>(
            morning::WAKE_UP >>
            RIDE_STATIONARY_BIKE >>
            selfcare::SHOWER
        )
    );
} 
```

The `MorningRoutine` `flow` now contains the functionality of three components, all without the `MorningRoutine` source
code having known about the new functionality. We can mix and match new components without modifying the original
source code.

```
    ┌─MorningRoutine─────────────┐
    │                            │
    │  WAKE_UP                   │
    │     │  │                   │
    │     │  ▼                   │
    │     │ RIDE_STATIONARY_BIKE │
    │     │  │                   │
    │     ▼  ▼                   │
    │  selfcare::SHOWER          │
    │     │                      │
    │     ▼                      │
    │  selfcare::GET_DRESSED     │
    │     │                      │
    │     ▼                      │
    │  food::MAKE_COFFEE         │
    │     │  │                   │
    │     │  ▼                   │
    │     │ PACK_SCHOOL_LUNCHES  │
    │     │  │                   │
    │     ▼  ▼                   │
    │  food::DRINK_COFFEE        │
    │     │                      │
    │     ▼                      │
    │  food::MAKE_BREAKFAST      │
    │     │                      │
    │     ▼                      │
    │  food::EAT_BREAKFAST       │
    │     │                      │
    │     ▼                      │
    │  SEND_KIDS_TO_SCHOOL       │
    │                            │
    └────────────────────────────┘
```

The `cib` library will take care of initializing the building all services, including `flow` services. For `flow`s, this
means the dependency graph will be serialized into a sequence of actions at compile-time to be executed in order at
runtime.

```
MorningRoutine
 1. WAKE_UP
 2. RIDE_STATIONARY_BIKE
 3. selfcare::SHOWER
 4. selfcare::GET_DRESSED
 5. food::MAKE_COFFEE
 6. PACK_SCHOOL_LUNCHES
 7. food::DRINK_COFFEE
 8. food::MAKE_BREAKFAST
 9. food::EAT_BREAKFAST
10. SEND_KIDS_TO_SCHOOL
```

All of these components are brought together in a project component:

```c++
// bring together all the components for the project
struct my_life {
    constexpr auto config = 
        cib::components<
            morning,
            childcare,
            exercise
        >;
}; 

// use cib::top to create our nexus and main function for us.
cib::top<my_life> top{};

int main() {
    top.main();
}
```

## Theory of Operation

While a flow is being defined during the constexpr init() phase, the flow::builder represents the actions and dependencies
as a directed acyclic graph. Each action is a node while a dependency between actions is a directed edge from one node
to another. During the build() phase, the graph is ordered into a sequence of steps using Kahn's Algorithm. Any circular
dependency results in a compilation error.

Because the flow library implements the constexpr init()/build() pattern, all the heavy lifting of registering actions
and resolving dependencies is computed at compile time. The built flow is an array of actions sized to exactly match the
number of actions. The flow code includes a directive to the compiler to always unroll the loop that executes the
actions. This actually results in a smaller code size and ultimately leads the compiler to inline every action into a
single function. The resulting flow code is just as efficient as if it were coded by hand in one big function.