#include <sc/string_constant.hpp>

namespace sc {
template <auto... chars> struct Log { constexpr static bool v = false; };

template <char... chars> void log(sc::string_constant<char, chars...>) {
    static_assert(Log<chars...>::v);
}

// create a string_constant
auto hi = "Hello, cruel world!"_sc;
[[maybe_unused]] auto empty = ""_sc;

// get a substring
auto sub = hi.substr(int_<0>, int_<5>);
static_assert(sub == "Hello"_sc);

// check for equality
static_assert("hi"_sc == "hi"_sc);
static_assert(!("hi"_sc == "hello"_sc));

static_assert("hi"_sc != "hello"_sc);
static_assert(!("hello"_sc != "hello"_sc));

// join multiple strings together
static_assert("Hello, "_sc + "cruel world!"_sc == "Hello, cruel world!"_sc);
static_assert(""_sc + "Luke"_sc == "Luke"_sc);
static_assert("Computer"_sc + ""_sc == "Computer"_sc);

// replace portions of a string
auto str = "Hello George, my name is Luke."_sc;
auto str2 = str.replace(int_<6>, int_<6>, "Frank"_sc);
static_assert(str2 == "Hello Frank, my name is Luke."_sc);

// convert strings to ints
static_assert(sc::to_int("0"_sc) == 0);
static_assert(sc::to_int("1"_sc) == 1);
static_assert(sc::to_int("10"_sc) == 10);
static_assert(sc::to_int("2345"_sc) == 2345);
static_assert(sc::to_int("2147483647"_sc) == 2147483647);
static_assert(sc::to_int("-1"_sc) == -1);
static_assert(sc::to_int("-732"_sc) == -732);
// static_assert(sc::to_int("-2147483648"_sc) == -2147483648);
static_assert(sc::to_int("-2147483647"_sc) == -2147483647);

// TODO: check for inconsistent format string and arguments
} // namespace sc
