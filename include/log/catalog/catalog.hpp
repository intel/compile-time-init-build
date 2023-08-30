#pragma once

#include <log/level.hpp>

#include <cstdint>

namespace sc {
template <typename...> struct args;
template <typename, typename T, T...> struct undefined;
} // namespace sc

template <logging::level level, typename MessageString> struct message {};

using string_id = std::uint32_t;

template <typename StringType> extern auto catalog() -> string_id;

template <typename StringType> inline auto catalog(StringType) -> string_id {
    return catalog<StringType>();
}
