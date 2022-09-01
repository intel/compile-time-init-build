# string_constant

An `sc::string_constant` is a compile-time string in the same way
that `std::integral_constant` is a compile-time integral value. When firmware
needs to emit a log message, an integral ID is emitted instead of the string
data. This conserves both runtime and storage resources. When decoding the log
messages, a catalog file is needed to map the IDs back to their string values.

There are two challenges with such a system. The first is assigning a unique ID
to each string value and the second is how to support rich string operations
like format and concatenation.

The `sc::string_constant` library solves both of these challenges by
representing the value of the string in its type. The `sc::string_constant`'s
string template struct is parameterized by a list of chars representing the
string value as well as a tuple of zero or more dynamic arguments to be
formatted by a log decoder.

The details of this implementation are provided in the Theory of Operation
section.

[![Embedded Logging Case Study: From C to Shining C++ - Luke Valenty -CppNow 2022](https://img.youtube.com/vi/Dt0vx-7e_B0/0.jpg)](https://www.youtube.com/watch?v=Dt0vx-7e_B0)


## Usage

### `_sc` user defined literal

`sc::string_constant` uses a user-defined literal to make it easy to define a
string:

```c++
constexpr auto hello_world = "Hello, World!"_sc; 
```

Note how the auto keyword is used for the string type. This is necessary because
the string's type changes depending on its value. It would be redundant and
burdensome to specify the exact type.

Also note the `constexpr` keyword is used. `sc::string_constant`s are fully
`constexpr` and nearly all operations are performed at compile-time even without
the `constexpr` keyword.

Because the string value is represented by its type, strings can be assigned as
type aliases or passed in as template parameters.

```c++
using HelloWorldType = decltype("Hello, World!"_sc);

constexpr HelloWorldType helloWorld{};

static_assert(helloWorld == "Hello, World!"_sc); 
```

### Concatenation

Two strings can be joined together using the `+` operator:

```c++
static_assert("Hi "_sc + "Emily!"_sc == "Hi Emily!"_sc); 
```

### Format

`sc::string_constant`s can be formatted using a subset of python
or `fmt::format`
format specifiers. Formatting behavior is different depending on whether the
arguments are compile-time values or dynamic values. Compile-time values will be
formatted at compile time while dynamic values will only be formatted by tools
outside of firmware like a log decoder.

```c++
constexpr auto my_age = 6; 
constexpr auto day_to_go = day_of_week::SATURDAY; 
auto my_name = "Olivia"_sc; 
using BirthdayParty = party<party_type::Birthday, day_of_week::TUESDAY>;

// use int_<value> to format an integer known at compile time 
static_assert(
    format("I am {} years old."_sc, sc::int_<my_age>) == 
    "I am 6 years old."_sc);

// use enum_<value> to format an enum value known at compile time
static_assert(
    format("Let's go on {}."_sc, sc::enum_<day_to_go>) == 
    "Let's go on SATURDAY."_sc);

// strings can be used as format arguments as well 
static_assert(
    format("My name is {}."_sc, my_name) == 
    "My name is Olivia."_sc);

// use type_<type> to get the string of a type which can then be used as a format argument 
static_assert(
    type_<BirthdayParty> == 
    "party<party_type::Birthday, day_of_week::TUESDAY>"_sc);

// multiple arguments can be formatted at once 
static_assert(
    format("My name is {} and I am {} years old."_sc, my_name, my_age) == 
    "My name is Olivia and I am 6 years old."_sc);

// runtime arguments not known at compile-time can also be formatted. the dynamic values can be 
// accessed at runtime and emitted along with the string_constant id. 
void read_memory(std::uint32_t addr) { 
    auto const my_message = format("Reading memory at {:08x}"_sc, addr); 
}

// both runtime and compile time arguments can be used in a single format operation 
template<typename RegType>
void readReg() { 
    RegType reg{}; 
    auto const reg_value = apply(read(reg.raw()));
    auto const my_message = format("Register {} = {}"_sc, type_<RegType>, reg_value); 
} 
```

Note that values known at compile time are passed into template variables as
template parameters. This allows the string.format method to access constexpr
versions of the values and format them into the string.

If you format integers or enums without using the template variables, then the
formatting happens outside of firmware by the log decoder collateral.

## Theory of Operation

Compile-time string literals are created using user-defined string template
literals. These are not yet part of a C++ standard but are supported by GCC,
Clang, and MSVC. They allow for easy conversion of a string literal to template
parameters.

```c++
template<class T, T... chars>
constexpr auto operator""_sc() {
    return sc::string_constant<T, chars...>{};
}
```

We can see clearly that the string value is encoded in the type.

```c++
// create a compile-time string. note that it does not need to be constexpr or even const because the value is encoded in the type. 
auto hello_value = "Hello!"_sc;
using HelloType = decltype(hello_value);

// the string's value is represented by its type 
using ExpectedType = sc::string_constant<char, 'H', 'e', 'l', 'l', 'o', '!'>;

ExpectedType expected_value{};

// same type 
static_assert(std::is_same_v<HelloType, ExpectedType>);

// same value 
static_assert(hello_value == expected_value); 
```

Encoding the string value in the type is necessary for string_constant support.
Type information is used when linking object files together. This means the
string values can be present in object files to be extracted during the build
process. The other part is assigning a unique ID to each string. This can be
done with the declaration of a template function with external linkage like
this:

```c++
template<typename StringType>
extern std::uint32_t get_string_id(StringType); 
```

When a string needs to be emitted as part of a log message, the `get_string_id()`
function is used to translate the string type to its ID. During compilation, an
intermediate object file is generated missing a definition for each
instantiation of the `get_string_id()` function. This information can be extracted
using the 'nm' tool and the original string values recovered. A
new `strings.cpp` file can be generated that implements all the template
instantiations returning a unique ID for each string. When the original object
binary is linked to `strings.o`, the calls to `get_string_id()` can be replaced
with the actual int value.

