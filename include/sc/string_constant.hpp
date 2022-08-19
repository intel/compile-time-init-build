#pragma once


#include <sc/fwd.hpp>
#include <sc/detail/string_meta.hpp>

#include <utility>
#include <string_view>
#include <limits>
#include <array>
#include <iterator>



namespace sc {
    template<typename StringViewConstant>
    [[nodiscard]] constexpr static auto create() noexcept {
        return detail::unpack_into_string_constant<StringViewConstant>(
            std::make_integer_sequence<size_type, StringViewConstant::value.size()>{}
        );
    }

    template<typename CharT, CharT... chars>
    struct string_constant {
    private:
        using This = string_constant<CharT, chars...>;
        using StringView = std::basic_string_view<CharT>;

        constexpr static CharT storage[sizeof...(chars)]{chars...};


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

        constexpr static StringView value{storage, sizeof...(chars)};
        constexpr static size_type npos = std::numeric_limits<size_type>::max();

        [[nodiscard]] constexpr operator StringView() const noexcept {
            return value;
        }

        [[nodiscard]] constexpr StringView operator()() const noexcept {
            return value;
        }

        constexpr static const_iterator begin() noexcept {
            return value.begin();
        }

        constexpr static const_iterator end() noexcept {
            return value.end();
        }

        [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept {
            static_assert(sizeof...(chars) > 0);
            return value[pos];
        }

        template<typename T, T pos>
        [[nodiscard]] constexpr auto operator[](std::integral_constant<T, pos>) const noexcept {
            static_assert(pos < sizeof...(chars));
            return value[pos];
        }

        [[nodiscard]] constexpr static auto size() noexcept {
            return value.size();
        }

        [[nodiscard]] constexpr static auto length() noexcept {
            return value.length();
        }

        [[nodiscard]] constexpr static auto empty() noexcept {
            return value.empty();
        }

        template<
            size_type pos = 0,
            size_type count = npos>
        [[nodiscard]] constexpr static auto substr(
            std::integral_constant<size_type, pos>,
            std::integral_constant<size_type, count>
        ) {
            return create<detail::SubStr<This, pos, count>>();
        }

        template<
            size_type pos,
            size_type count,
            typename StrT>
        [[nodiscard]] constexpr static auto replace(
            std::integral_constant<size_type, pos>,
            std::integral_constant<size_type, count>,
            StrT
        ) noexcept {
            return create<detail::Replace<This, pos, count, StrT>>();
        }

        [[nodiscard]] constexpr static uint64_t hash() {
            // http://www.cse.yorku.ca/~oz/hash.html @ Aug. 19, 2022
            // this is a very slightly cleaned up version of djb2
            uint64_t hash = 5381;

            for (uint64_t const c :  value) {
                hash = hash * 33 + c;
            }

            return hash;
        }
    };

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator==(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return false;
    }

    template<class CharT, CharT... chars>
    [[nodiscard]] constexpr auto operator==(
        string_constant<CharT, chars...> lhs,
        string_constant<CharT, chars...> rhs
    ) noexcept {
        return true;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator!=(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return true;
    }

    template<class CharT, CharT... chars>
    [[nodiscard]] constexpr auto operator!=(
        string_constant<CharT, chars...> lhs,
        string_constant<CharT, chars...> rhs
    ) noexcept {
        return false;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator<(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return lhs.value < rhs.value;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator<=(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return lhs.value <= rhs.value;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator>(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return lhs.value > rhs.value;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator>=(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return lhs.value >= rhs.value;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator+(
        string_constant<CharT, charsLhs...> lhs,
        string_constant<CharT, charsRhs...> rhs
    ) noexcept {
        return string_constant<CharT, charsLhs..., charsRhs...>{};
    }

    template<
        typename CharT,
        CharT... chars>
    [[nodiscard]] constexpr int to_int(
        string_constant<CharT, chars...> strc
    ) noexcept {
        int value = 0;
        bool negative = false;
        std::basic_string_view<CharT> str = strc.value;

        while (str.size() > 0) {
            if (str.front() == '-') {
                negative = true;
                str.remove_prefix(1);
            } else {
                value *= 10;
                value += str.front() - '0';
                str.remove_prefix(1);
            }
        }

        value = negative ? -value : value;

        return value;
    }
}
