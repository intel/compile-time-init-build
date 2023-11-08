# Flow

A flow is a series of actions to be executed in order based on their dependencies. The flow library was originally
designed to be used for power flows, moving from one power state to another. It can also be used as a generic extension
point to register callbacks at compile-time. Flow implements the constexpr init()/build() pattern.

## Example 

Flow extension points are declared by extending the `flow::service`.

```c++
struct MorningRoutine : public flow::service<"MorningRoutine"> {}; 
```

The builder is used during the constexpr init() phase to define and extend the flow. Actions to be added to the flow are
declared and defined as constexpr constants. all_t actions added to a flow will be executed.

```c++
namespace food { 
    constexpr static auto MAKE_COFFEE = flow::action<"MAKE_COFFEE">([] { 
        coffee_maker.start(ground_coffee.take(100_grams), water.take(16_ounces)); 
    }); 
    
    constexpr static auto DRINK_COFFEE = flow::action<"DRINK_COFFEE">([] { 
        ...
    }); 
} 
```

Actions are added to the flow inside a component's `cib::config`.

```c++
struct morning {
    constexpr auto config = cib::config(
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
    constexpr static auto PACK_SCHOOL_LUNCHES = flow::action<"PACK_SCHOOL_LUNCHES">([] { 
        ...
    }); 
    
    constexpr static auto SEND_KIDS_TO_SCHOOL = flow::action<"SEND_KIDS_TO_SCHOOL">([] { 
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
    constexpr static auto RIDE_STATIONARY_BIKE = flow::action<"RIDE_STATIONARY_BIKE">([] { 
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

all_t of these components are composed in a project component and brought to life with an instance of `cib::top`. We need
to make sure our `flow`s get executed at the appropriate times, so we our example has a `day_cycle` component that 
defines the various extension points and ensures they get executed over and over in `cib::top`'s `MainLoop`.


```c++
// simple component for scheduling daily activities.
struct day_cycle {
    constexpr static auto DAY_CYCLE = flow::action<"DAY_CYCLE">([] { 
        flow::run<MorningRoutine>();
        flow::run<DaytimeRoutine>();
        flow::run<EveningRoutine>();
        wait_for_morning_time();
    }); 
    
    constexpr auto config = cib::config(
        cib::exports<
            MorningRoutine, 
            DaytimeRoutine, 
            EveningRoutine
        >,
        
        cib::extend<MainLoop>(
            DAY_CYCLE
        )
    );
} 

// bring together all the components for the project
struct my_life {
    constexpr auto config = 
        cib::components<
            day_cycle,
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

## API

### `flow::service`

Define a new `flow` service. If the `flow::service` template type is given a `sc::string_constant` name then it will
automatically log the beginning and end of the `flow` as well as all actions.

#### Example

```c++
// declare a flow without logging
struct MyFlow : public flow::service<> {};

// declare a flow with automatic logging enabled
struct MyFlowWithLogging : public flow::service<"MyFlowWithLogging"> {}; 
```

### `flow::action`

Define a new `flow` action. all_t `flow` actions are created with a name and lambda. `flow` action and milestone names 
must be unique within a `flow`. The same action can be used in multiple flows. Actions cannot be added to a flow more
than once, but can be referenced by other actions when adding dependencies.

#### Example

```c++
constexpr static auto MY_ACTION_NAME = flow::action<"MY_ACTION_NAME">([] {
    // do useful stuff
}); 
```

### `flow::milestone`

Define a new `flow` milestone. Milestones only have a name and perform no action. They are used as well-defined points
within a `flow` in which other actions may base their dependencies on.

#### Example

```c++
constexpr static auto MY_MILESTONE_NAME = flow::milestone<"MY_MILESTONE_NAME">(); 
```

### `flow::run`

#### Example

```c++
flow::run<MyFlow>();
```

### `>>`

Create a dependency between two or more actions and/or milestones. Must be passed into the `cib::extend` configuration
method for it to have an effect. Can be changed together to create a sequence of dependent actions.

#### Example

```c++
namespace example_component {
    constexpr auto config = cib::config(
        cib::extend<MyFlow>(
            // SOME_ACTION must execute before SOME_OTHER_ACTION
            SOME_ACTION >> SOME_OTHER_ACTION
        )
    );
}
```

### `&&`

Allow two or more actions and/or milestones to be added in parallel without any ordering requirement between them. If
there is no dependency between two or more actions, this is the preferred way of adding them to a `flow`. Other 
components will then be able to insert actions in between if needed.

#### Example

```c++
namespace example_component {
    constexpr auto config = cib::config(
        cib::extend<MyFlow>(
            // no order requirement between these actions
            SOME_ACTION && SOME_OTHER_ACTION
        )
    );
}
```


## Theory of Operation

While a flow is being defined during the constexpr init() phase, the flow::builder represents the actions and dependencies
as a directed acyclic graph. Each action is a node while a dependency between actions is a directed edge from one node
to another. During the build() phase, the graph is ordered into a sequence of steps using Kahn's Algorithm. any_t circular
dependency results in a compilation error.

Because the flow library implements the constexpr init()/build() pattern, all the heavy lifting of registering actions
and resolving dependencies is computed at compile time. The built flow is an array of actions sized to exactly match the
number of actions. The flow code includes a directive to the compiler to always unroll the loop that executes the
actions. This actually results in a smaller code size and ultimately leads the compiler to inline every action into a
single function. The resulting flow code is just as efficient as if it were coded by hand in one big function.
