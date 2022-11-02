#pragma once

#include <cib/detail/meta.hpp>
#include <cib/tuple.hpp>
#include <sc/detail/conversions.hpp>
#include <sc/detail/format_spec.hpp>
#include <sc/lazy_string_format.hpp>
#include <sc/string_constant.hpp>

#include <array>
#include <string_view>
#include <type_traits>

namespace sc {
struct repl_field_iter {
    std::string_view fmt_;
    std::string_view::const_iterator i;

    constexpr repl_field_iter operator++() {
        while (i != fmt_.cend() && *i != '{') {
            i++;
        }

        // advance just after the '{'
        if (i != fmt_.cend()) {
            i++;
        }

        return *this;
    }

    [[nodiscard]] constexpr std::string_view operator*() const {
        auto end = i;
        while (*end != '}' && end != fmt_.cend()) {
            end++;
        }

        return std::string_view(i, std::distance(i, end));
    }

    [[nodiscard]] constexpr bool operator==(repl_field_iter other) const {
        return i == other.i;
    }

    [[nodiscard]] constexpr bool operator!=(repl_field_iter other) const {
        return i != other.i;
    }
};

struct repl_fields {
    std::string_view fmt;

    [[nodiscard]] constexpr repl_field_iter begin() const {
        return ++repl_field_iter{fmt, fmt.begin()};
    }
    [[nodiscard]] constexpr repl_field_iter end() const {
        return repl_field_iter{fmt, fmt.end()};
    }
};

template <typename InputIter, typename OutputIter>
constexpr OutputIter copy(InputIter in_iter, InputIter in_end,
                          OutputIter out_iter) {
    while (in_iter != in_end) {
        *out_iter++ = *in_iter++;
    }

    return out_iter;
}

template <typename CharT, CharT... chars>
[[nodiscard]] constexpr char *format_field(std::string_view field,
                                           string_constant<CharT, chars...> arg,
                                           char *out) {
    return copy(arg.begin(), arg.end(), out);
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] constexpr char *format_field(std::string_view field, T,
                                           char *out) {
    return copy(field.begin() - 1, field.end() + 1, out);
}

template <typename CharT, CharT... chars, typename ArgsTupleT>
[[nodiscard]] constexpr char *format_field(
    std::string_view field,
    lazy_string_format<string_constant<CharT, chars...>, ArgsTupleT> lazy,
    char *out) {
    return copy(lazy.str.begin(), lazy.str.end(), out);
}

template <typename EnumTypeT, EnumTypeT ValueT,
          std::enable_if_t<std::is_enum<EnumTypeT>::value, bool> = true>
[[nodiscard]] constexpr char *
format_field(std::string_view field, std::integral_constant<EnumTypeT, ValueT>,
             char *out) {
    auto const &enum_sv = detail::EnumToString<EnumTypeT, ValueT>::value;
    return copy(enum_sv.begin(), enum_sv.end(), out);
}

template <typename T>
[[nodiscard]] constexpr char *format_field(std::string_view field, type_name<T>,
                                           char *out) {
    auto const &type_name_sv = detail::TypeNameToString<T>::value;
    return copy(type_name_sv.begin(), type_name_sv.end(), out);
}

template <typename IntegralTypeT, IntegralTypeT ValueT,
          std::enable_if_t<!std::is_enum<IntegralTypeT>::value, bool> = true>
[[nodiscard]] constexpr char *
format_field(std::string_view field,
             std::integral_constant<IntegralTypeT, ValueT> const &, char *out) {
    detail::fast_format_spec spec{field, 0};

    auto const base = [&]() {
        if (spec.type == 'b') {
            return 2;
        } else if (spec.type == 'o') {
            return 8;
        } else if (spec.type == 'x' || spec.type == 'X') {
            return 16;
        } else {
            return 10;
        }
    }();

    bool const uppercase = spec.type == 'X';

    auto const int_static_string =
        detail::integral_to_string(ValueT, base, uppercase);

    if (spec.padding_width > 0) {
        auto const padding_width =
            std::max<int>(spec.padding_width - int_static_string.count, 0);

        auto const pad_char = spec.zero_pad ? '0' : ' ';

        for (int i = 0; i < padding_width; i++) {
            *out++ = pad_char;
        }
    }

    std::string_view const int_sv = int_static_string;
    return copy(int_sv.begin(), int_sv.end(), out);
}

struct format_buf_result {
    std::array<char, 2000> data;
    std::size_t size;
};

template <typename FmtStringConstant, typename... ArgTs> struct format_t {
    // FIXME: calculate buffer size based on input
    constexpr static format_buf_result buf = []() {
        format_buf_result tmp_buf{};

        auto const fmt = FmtStringConstant::value;
        repl_fields fields{fmt};

        auto out = tmp_buf.data.begin();
        auto in = fmt.begin();
        auto field_iter = fields.begin();

        (([&](auto arg) {
             auto const f = *field_iter;
             ++field_iter;

             out = copy(in, f.begin() - 1, out); // copy before the field
             out = format_field(f, arg, out);
             in = f.end() + 1;
         }(ArgTs{})),
         ...);

        out = copy(in, fmt.end(), out);

        tmp_buf.size = std::distance(tmp_buf.data.begin(), out);

        return tmp_buf;
    }();

    constexpr static std::string_view value{buf.data.begin(), buf.size};
};

template <typename T>
struct is_lazy_format_string : public std::integral_constant<bool, false> {};

template <typename CharT, CharT... chars, typename ArgsTupleT>
struct is_lazy_format_string<
    lazy_string_format<string_constant<CharT, chars...>, ArgsTupleT>>
    : public std::integral_constant<bool, true> {};

template <typename T>
constexpr is_lazy_format_string<T> is_lazy_format_string_v{};

template <typename T>
constexpr bool is_lazy_format_string_with_args_v = []() {
    if constexpr (is_lazy_format_string_v<T>) {
        return T::has_args;
    } else {
        return false;
    }
};

template <typename CharT, CharT... chars, typename... ArgTs>
[[nodiscard]] constexpr auto format(string_constant<CharT, chars...> fmtStr,
                                    ArgTs... args) {
    auto const runtime_args = [&]() {
        constexpr bool has_runtime_args = []() {
            constexpr bool has_integral_args =
                (std::is_integral_v<ArgTs> || ...);

            if constexpr (has_integral_args) {
                return true;

            } else {
                constexpr bool has_lazy_args =
                    (is_lazy_format_string_v<ArgTs> || ...);

                if constexpr (has_lazy_args) {
                    return (is_lazy_format_string_with_args_v<ArgTs> || ...);
                } else {
                    return false;
                }
            }
        }();

        if constexpr (has_runtime_args) {
            return cib::make_tuple(args...).fold_right(
                cib::make_tuple(), [](auto arg, auto state) {
                    if constexpr (std::is_integral_v<decltype(arg)>) {
                        return cib::tuple_cat(cib::make_tuple(arg), state);

                    } else if constexpr (is_lazy_format_string_v<decltype(
                                             arg)>) {
                        return cib::tuple_cat(arg.args, state);

                    } else {
                        return state;
                    }
                });

        } else {
            return cib::make_tuple();
        }
    }();

    return lazy_string_format{
        create<format_t<string_constant<CharT, chars...>, ArgTs...>>(),
        runtime_args};
}

} // namespace sc