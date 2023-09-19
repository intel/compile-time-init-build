# logging

Logging in *cib* is in two parts:
- the interface, in [log.hpp](log.hpp)
- an implementation, which can be specified at the top level

Three possible logger implementations are provided:
- one using libfmt in [fmt/logger.hpp](fmt/logger.hpp)
- one using the [MIPI SyS-T spec](https://www.mipi.org/specifications/sys-t), in [catalog/mipi_encoder.hpp](catalog/mipi_encoder.hpp)
- the null logger (accepts everything, never produces output)

## log levels

*cib* offers 6 well-known and 2 user-defined log levels, according to the [MIPI SyS-T spec](https://www.mipi.org/specifications/sys-t).
```c++
namespace logging {
enum level {
    MAX = 0,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    INFO = 4,
    USER1 = 5,
    USER2 = 6,
    TRACE = 7
};
}
```

## interface

*cib* logging macros follow the log levels available:
```cpp
CIB_TRACE(...);
CIB_INFO(...);
CIB_WARN(...);
CIB_ERROR(...);
CIB_FATAL(...);
```
These are the same as calling `CIB_LOG` with the `logging::level` as the first argument.

`CIB_FATAL(...)` also causes a program termination according to the logging implementation.
`CIB_ASSERT(expression)` is also available and calls `CIB_FATAL` if the asserted expression is false.

In order to use logging in a header, it suffices only to include
[log.hpp](log.hpp) and use the macros. Header-only clients of logging do not
need to know the implementation selected.

To use logging in a translation unit, the TU needs to see a customization, which brings us to...

## selecting a logger

Programs can choose which logger to use by specializing the `logging::config` variable template.
Left unspecialized, the null logger will be used.

```cpp
// use libfmt logging to std::cout
template <>
inline auto logging::config<> = logging::fmt::config{std::ostream_iterator<char>{std::cout}};
```

The provided `libfmt` implementation can output to multiple destinations by constructing
`logging::fmt::config` with multiple `ostream` iterators.

***NOTE:*** Be sure that each translation unit sees the same specialization of
`logging::config<>`! Otherwise you will have an [ODR](https://en.cppreference.com/w/cpp/language/definition) violation.

## implementing a logger

Each logging implementation (configuration) provides a customization point: a
`logger` object, which must implement `log`.
Therefore providing a custom implementation is a matter of defining this
structure appropriately.

```cpp
namespace my_logger {
struct config {
  struct {
    template <logging::level L, typename... Ts>
    auto log(Ts &&...ts) -> void {
      // log the ts... according to my mechanism
    }
  } logger;
};
}
```

To use the custom implementation, specialize `logging::config`:
```cpp
// use my logger
template <>
inline auto logging::config<> = my_logger::config{};
```

*cib* uses a libfmt-inspired convention for logging: the first argument to `log` is a format string with `{}` as format placeholders.
