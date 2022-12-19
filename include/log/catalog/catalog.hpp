#pragma once

#include <log/level.hpp>

#include <string_view>

template <logging::level level, typename MessageString> struct message {};

using string_id = std::uint32_t;

template <typename StringType> extern auto catalog() -> string_id;

template <typename StringType> inline auto catalog(StringType) -> string_id {
    return catalog<StringType>();
}
