#pragma once

#include <sc/fwd.hpp>

#include <string_view>
#include <utility>

namespace sc::detail {
using size_type = int;

template <typename This, size_type pos, size_type count> struct SubStr {
    using char_type = typename This::char_type;
    constexpr static std::basic_string_view<char_type> value =
        This::value.substr(pos, count);
};

template <typename StringViewConstant>
[[nodiscard]] constexpr static auto create() noexcept {
    using value_type = typename decltype(StringViewConstant::value)::value_type;
    return [&]<size_type... Is>(std::integer_sequence<size_type, Is...>) {
        return string_constant<value_type, StringViewConstant::value[Is]...>{};
    }
    (std::make_integer_sequence<size_type, StringViewConstant::value.size()>{});
}
} // namespace sc::detail
