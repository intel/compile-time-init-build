#pragma once


#include <sc/fwd.hpp>
#include <sc/string_constant.hpp>


namespace sc {
    template<
        typename StringConstantT,
        typename ArgTupleT>
    struct lazy_string_format {
        constexpr static StringConstantT str{};
        constexpr static bool has_args = std::tuple_size_v<ArgTupleT> > 0;
        ArgTupleT args;

        constexpr lazy_string_format()
            : args{}
        {}

        constexpr lazy_string_format(
            StringConstantT,
            ArgTupleT newArgs
        )
            : args{newArgs}
        {}
    };

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator==(
        lazy_string_format<string_constant<CharT, charsLhs...>, std::tuple<>>,
        string_constant<CharT, charsRhs...>
    ) noexcept {
        return bool_<false>;
    }

    template<class CharT, CharT... chars>
    [[nodiscard]] constexpr auto operator==(
        lazy_string_format<string_constant<CharT, chars...>, std::tuple<>>,
        string_constant<CharT, chars...>
    ) noexcept {
        return bool_<true>;
    }

    template<
        class CharT,
        CharT... charsLhs,
        typename ArgsTupleLhsT,
        CharT... charsRhs>
    [[nodiscard]] constexpr auto operator==(
        lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT>,
        string_constant<CharT, charsRhs...>
    ) noexcept {
        return bool_<false>;
    }

    template<class CharT, CharT... charsLhs, CharT... charsRhs>
    [[nodiscard]] constexpr auto operator!=(
        lazy_string_format<string_constant<CharT, charsLhs...>, std::tuple<>>,
        string_constant<CharT, charsRhs...>
    ) noexcept {
        return bool_<true>;
    }

    template<class CharT, CharT... chars>
    [[nodiscard]] constexpr auto operator!=(
        lazy_string_format<string_constant<CharT, chars...>, std::tuple<>>,
        string_constant<CharT, chars...>
    ) noexcept {
        return bool_<false>;
    }

    template<
        class CharT,
        CharT... charsLhs,
        typename ArgsTupleLhsT,
        CharT... charsRhs>
    [[nodiscard]] constexpr auto operator!=(
        lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT>,
        string_constant<CharT, charsRhs...>
    ) noexcept {
        return bool_<true>;
    }



    template<
        class CharT,
        CharT... charsLhs,
        typename ArgsTupleLhsT,
        CharT... charsRhs,
        typename ArgsTupleRhsT>
    [[nodiscard]] constexpr bool operator==(
        lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT> lhs,
        lazy_string_format<string_constant<CharT, charsRhs...>, ArgsTupleRhsT> rhs
    ) noexcept {
        return (lhs.str == rhs.str) && (lhs.args == rhs.args);
    }

    template<
        class CharT,
        CharT... charsLhs,
        typename ArgsTupleLhsT,
        CharT... charsRhs,
        typename ArgsTupleRhsT>
    [[nodiscard]] constexpr bool operator!=(
        lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT> lhs,
        lazy_string_format<string_constant<CharT, charsRhs...>, ArgsTupleRhsT> rhs
    ) noexcept {
        return !(lhs == rhs);
    }


    template<
        typename StringConstantLhsT,
        typename TupleArgsLhsT,
        typename StringConstantRhsT,
        typename TupleArgsRhsT>
    [[nodiscard]] constexpr auto operator+(
        lazy_string_format<StringConstantLhsT, TupleArgsLhsT> lhs,
        lazy_string_format<StringConstantRhsT, TupleArgsRhsT> rhs
    ) noexcept {
        return lazy_string_format{
            lhs.str + rhs.str,
            std::tuple_cat(lhs.args, rhs.args)};
    }

    template<
        typename StringConstantLhsT,
        typename TupleArgsLhsT,
        typename CharT,
        CharT... chars>
    [[nodiscard]] constexpr auto operator+(
        lazy_string_format<StringConstantLhsT, TupleArgsLhsT> lhs,
        string_constant<CharT, chars...> rhs
    ) noexcept {
        return lazy_string_format{
            lhs.str + rhs,
            lhs.args};
    }

    template<
        typename CharT,
        CharT... chars,
        typename StringConstantRhsT,
        typename TupleArgsRhsT>
    [[nodiscard]] constexpr auto operator+(
        string_constant<CharT, chars...> lhs,
        lazy_string_format<StringConstantRhsT, TupleArgsRhsT> rhs
    ) noexcept {
        return lazy_string_format{
            lhs + rhs.str,
            rhs.args};
    }


    template<
        typename StringConstantLhsT,
        typename StringConstantRhsT>
    [[nodiscard]] constexpr auto operator+(
        lazy_string_format<StringConstantLhsT, std::tuple<>> lhs,
        lazy_string_format<StringConstantRhsT, std::tuple<>> rhs
    ) noexcept {
        return lhs.str + rhs.str;
    }

    template<
        typename StringConstantLhsT,
        typename CharT,
        CharT... chars>
    [[nodiscard]] constexpr auto operator+(
        lazy_string_format<StringConstantLhsT, std::tuple<>> lhs,
        string_constant<CharT, chars...> rhs
    ) noexcept {
        return lhs.str + rhs;
    }

    template<
        typename CharT,
        CharT... chars,
        typename StringConstantRhsT>
    [[nodiscard]] constexpr auto operator+(
        string_constant<CharT, chars...> lhs,
        lazy_string_format<StringConstantRhsT, std::tuple<>> rhs
    ) noexcept {
        return lhs + rhs.str;
    }
}