#pragma once

#include <sc/fwd.hpp>

#include <array>
#include <iterator>
#include <limits>
#include <string_view>
#include <utility>

namespace sc {
template <typename CharT, CharT... chars> struct string_constant {
  private:
    using StringView = std::basic_string_view<CharT>;

    constexpr static std::array<CharT, sizeof...(chars)> storage{chars...};

  public:
    using char_type = CharT;
    using traits_type = typename StringView::traits_type;
    using value_type = typename StringView::value_type;
    using pointer = typename StringView::pointer;
    using const_pointer = typename StringView::const_pointer;
    using reference = typename StringView::reference;
    using const_reference = typename StringView::const_reference;
    using const_iterator = typename StringView::const_iterator;
    using iterator = typename StringView::const_iterator;
    using const_reverse_iterator = typename StringView::const_reverse_iterator;
    using reverse_iterator = typename StringView::const_reverse_iterator;
    using size_type = int;
    using difference_type = int;

    constexpr static StringView value{storage.data(), sizeof...(chars)};
    constexpr static size_type npos = std::numeric_limits<size_type>::max();

    // NOLINTNEXTLINE(google-explicit-constructor)
    [[nodiscard]] constexpr operator StringView() const noexcept {
        return value;
    }

    [[nodiscard]] constexpr auto operator()() const noexcept -> StringView {
        return value;
    }

    constexpr static auto begin() noexcept -> const_iterator {
        return value.begin();
    }

    constexpr static auto end() noexcept -> const_iterator {
        return value.end();
    }

    [[nodiscard]] constexpr auto operator[](size_type pos) const noexcept
        -> const_reference {
        static_assert(sizeof...(chars) > 0);
        return value[pos];
    }

    template <typename T, T pos>
    [[nodiscard]] constexpr auto
    operator[](std::integral_constant<T, pos>) const noexcept {
        static_assert(pos < sizeof...(chars));
        return value[pos];
    }

    [[nodiscard]] constexpr static auto size() noexcept { return value.size(); }

    [[nodiscard]] constexpr static auto length() noexcept {
        return value.length();
    }

    [[nodiscard]] constexpr static auto empty() noexcept {
        return value.empty();
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
