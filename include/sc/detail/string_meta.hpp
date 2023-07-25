#pragma once

#include <sc/fwd.hpp>

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

namespace sc::detail {
using size_type = int;

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
        std::array<char_type, size> buffer{};
        auto dst = buffer.begin();

        auto const first = thisStr.begin() + pos;
        auto const last = first + count;

        dst = std::copy(thisStr.begin(), first, dst);
        dst = std::copy(str.begin(), str.end(), dst);
        std::copy(last, thisStr.end(), dst);

        return buffer;
    }();

    constexpr static StringView value{storage.data(), size};
};

template <typename StringViewConstant>
[[nodiscard]] constexpr static auto create() noexcept {
    using value_type = typename decltype(StringViewConstant::value)::value_type;
    return [&]<size_type... Is>(std::integer_sequence<size_type, Is...>) {
        return string_constant<value_type, StringViewConstant::value[Is]...>{};
    }(std::make_integer_sequence<size_type,
                                 StringViewConstant::value.size()>{});
}
} // namespace sc::detail
