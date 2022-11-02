#pragma once

#include <log/log_level.hpp>

#include <string_view>

template <log_level level, typename MessageString> struct message {};

using string_id = std::uint32_t;

template <typename StringType> extern string_id catalog();

template <typename StringType> inline string_id catalog(StringType) {
    return catalog<StringType>();
}