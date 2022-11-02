#pragma once

#include <sc/fwd.hpp>

#include <array>
#include <string_view>
#include <utility>

namespace sc::detail {
template <typename StringViewConstant, size_type... indices>
[[nodiscard]] constexpr static auto unpack_into_string_constant(
    std::integer_sequence<size_type, indices...>) noexcept {
    using value_type = typename decltype(StringViewConstant::value)::value_type;
    return string_constant<value_type, StringViewConstant::value[indices]...>{};
}

template <typename This, size_type pos, size_type count> struct SubStr {
    using char_type = typename This::char_type;
    constexpr static std::basic_string_view<char_type> value =
        This::value.substr(pos, count);
};

template <typename This, size_type pos, size_type count, typename StrT>
struct Replace {
    using char_type = typename This::char_type;
    using StringView = std::basic_string_view<char_type>;

    constexpr static This thisStr{};
    constexpr static StrT str{};
    constexpr static int size = thisStr.length() - count + str.length();
    constexpr static std::array<char_type, size> storage = []() {
        // NOTE: use algorithms when moving to c++20

        std::array<char_type, size> buffer{};
        char *dst = buffer.begin();

        auto const first = thisStr.begin() + pos;
        auto const last = first + count;

        // copy begin() until first
        for (auto src = thisStr.begin(); src != first; src++, dst++) {
            *dst = *src;
        }

        // copy str.begin() until str.end()
        for (auto src = str.begin(); src != str.end(); src++, dst++) {
            *dst = *src;
        }

        // copy last until end()
        for (auto src = last; src != thisStr.end(); src++, dst++) {
            *dst = *src;
        }

        return buffer;
    }();

    constexpr static StringView value{storage.data(), size};
};
} // namespace sc::detail