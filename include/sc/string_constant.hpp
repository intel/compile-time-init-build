#pragma once

#include <sc/fwd.hpp>

#include <array>
#include <iterator>
#include <limits>
#include <string_view>
#include <utility>

namespace sc {
template <typename CharT, CharT... chars> struct string_constant {
    using value_type = std::basic_string_view<CharT>;

  private:
    constexpr static std::array<CharT, sizeof...(chars)> storage{chars...};

    using size_type = int;
    using const_iterator = typename value_type::const_iterator;
    constexpr static size_type npos = std::numeric_limits<size_type>::max();

  public:
    constexpr static value_type value{storage.data(), sizeof...(chars)};

    constexpr static auto begin() noexcept { return std::cbegin(storage); }
    constexpr static auto end() noexcept { return std::cend(storage); }

    [[nodiscard]] constexpr static auto size() noexcept {
        return std::size(storage);
    }

    template <size_type pos = 0, size_type count = npos>
    [[nodiscard]] constexpr static auto
    substr(std::integral_constant<size_type, pos>,
           std::integral_constant<size_type, count> = {}) {
        constexpr size_type sz = count == npos ? size() - pos : count;
        return [&]<size_type... Is>(std::integer_sequence<size_type, Is...>) {
            return string_constant<CharT, storage[pos + Is]...>{};
        }(std::make_integer_sequence<size_type, sz>{});
    }

    template <typename F> constexpr auto apply(F &&f) const {
        return std::forward<F>(f)(*this);
    }
};

template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator==(string_constant<CharT, charsLhs...>,
           string_constant<CharT, charsRhs...>) noexcept -> bool {
    return false;
}

template <class CharT, CharT... chars>
[[nodiscard]] constexpr auto
operator==(string_constant<CharT, chars...>,
           string_constant<CharT, chars...>) noexcept -> bool {
    return true;
}

#if __cpp_lib_three_way_comparison < 201907L
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator!=(string_constant<CharT, charsLhs...> lhs,
           string_constant<CharT, charsRhs...> rhs) noexcept -> bool {
    return not(lhs == rhs);
}
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator<(string_constant<CharT, charsLhs...> lhs,
          string_constant<CharT, charsRhs...> rhs) noexcept -> bool {
    return lhs.value < rhs.value;
}
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator>(string_constant<CharT, charsLhs...> lhs,
          string_constant<CharT, charsRhs...> rhs) noexcept -> bool {
    return lhs.value > rhs.value;
}
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator<=(string_constant<CharT, charsLhs...> lhs,
           string_constant<CharT, charsRhs...> rhs) noexcept -> bool {
    return lhs.value <= rhs.value;
}
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator>=(string_constant<CharT, charsLhs...> lhs,
           string_constant<CharT, charsRhs...> rhs) noexcept -> bool {
    return lhs.value >= rhs.value;
}
#else
template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator<=>(string_constant<CharT, charsLhs...> lhs,
            string_constant<CharT, charsRhs...> rhs) noexcept {
    return lhs.value <=> rhs.value;
}
#endif

template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto
operator+(string_constant<CharT, charsLhs...>,
          string_constant<CharT, charsRhs...>) noexcept
    -> string_constant<CharT, charsLhs..., charsRhs...> {
    return {};
}
} // namespace sc
