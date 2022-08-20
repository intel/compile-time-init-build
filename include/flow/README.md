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
declared and defined as constexpr constants.

```c++
namespace food { 
    constexpr static auto MAKE_COFFEE = flow::action("MAKE_COFFEE"_sc, [] { 
        coffee_maker.start(ground_coffee.take(100_grams), water.take(16_ounces)); 
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

Flows can be extended by inserting additional actions with new dependencies.

```c++
struct childcare { 
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

Multiple independent features can add actions to the same flow.

```c++
namespace exercise { 
    constexpr auto config = cib::config(
        cib::extend<MorningRoutine>(
            morning::WAKE_UP >>
            RIDE_STATIONARY_BIKE >>
            selfcare::SHOWER
        )
    );
} 
```

After the constexpr init() phase has completed, the flow builder can be built into a Flow.

```c++
// perform the constexpr init() using an immediately invoked lambda expression (IILE)
struct my_life {
    constexpr auto config = cib::components<
        morning,
        childcare,
        exercise
    >;
}; 

// build the morning routine flow. the size (number of actions) is fed back into the build function 
// as a template parameter so the storage size of the flow is optimized. however, the compiler will 
// unroll the loop of actions and inline all the action functions into a single function. 
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