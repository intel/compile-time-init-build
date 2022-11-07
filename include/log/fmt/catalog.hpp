#pragma once

#include <log/log_level.hpp>

#include <string_view>

template <log_level levelT, typename MessageStringT> struct message {
    static constexpr auto value = MessageStringT::str.value;
    static constexpr auto level = levelT;
};

using string_id = std::string_view;

template <typename StringType> [[nodiscard]] auto catalog() -> string_id {
    return StringType::value;
}

template <typename StringType>
[[nodiscard]] auto catalog(StringType) -> string_id {
    return catalog<StringType>();
}
