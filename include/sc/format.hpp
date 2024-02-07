#pragma once

#include <sc/fwd.hpp>
#include <sc/lazy_string_format.hpp>
#include <sc/string_constant.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_conversions.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace sc {
namespace detail {
template <typename T>
concept compile_time_field =
    std::same_as<typename T::value_type,
                 std::remove_cvref_t<decltype(T::value)>>;

template <compile_time_field T> [[nodiscard]] CONSTEVAL auto field_value(T) {
    if constexpr (std::is_enum_v<typename T::value_type>) {
        return stdx::enum_as_string<T::value>();
    } else {
        return T::value;
    }
}

template <typename T>
[[nodiscard]] CONSTEVAL auto field_value(sc::type_name<T>) {
    return stdx::type_as_string<T>();
}

template <typename Fmt, typename Arg> constexpr auto format1(Fmt, Arg arg) {
    constexpr auto str = [&] {
        constexpr auto fmtstr = FMT_COMPILE(Fmt::value);
        constexpr auto sz = fmt::formatted_size(fmtstr, field_value(arg));
        std::array<char, sz> buf{};
        fmt::format_to(std::begin(buf), fmtstr, field_value(arg));
        return buf;
    }();
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return string_constant<char, str[Is]...>{};
    }(std::make_index_sequence<std::size(str)>{});
}

template <typename Fmt> constexpr auto split_format_spec() {
    constexpr Fmt fmt{};
    constexpr auto spec_start = std::adjacent_find(
        std::begin(fmt), std::end(fmt),
        [](auto c1, auto c2) { return c1 == '{' and c2 != '{'; });
    if constexpr (spec_start == std::end(fmt)) {
        return std::pair{fmt, ""_sc};
    } else {
        constexpr auto spec_end = std::find_if(spec_start, std::end(fmt),
                                               [](auto c) { return c == '}'; });
        constexpr auto len = std::distance(std::begin(fmt), spec_end) + 1;
        return std::pair{fmt.substr(int_<0>, int_<len>), fmt.substr(int_<len>)};
    }
}

template <typename Str, typename Fmt, typename RuntimeTuple, typename Arg>
constexpr auto process_arg(stdx::tuple<Str, Fmt, RuntimeTuple> t, Arg arg) {
    using namespace stdx::literals;
    constexpr auto p = split_format_spec<Fmt>();
    if constexpr (requires { field_value(arg); }) {
        return stdx::make_tuple(t[0_idx] + format1(p.first, arg), p.second,
                                t[2_idx]);
    } else if constexpr (requires { arg.args; }) {
        return stdx::make_tuple(t[0_idx] + format1(p.first, arg.str), p.second,
                                stdx::tuple_cat(t[2_idx], arg.args));
    } else {
        return stdx::make_tuple(
            t[0_idx] + p.first, p.second,
            stdx::tuple_cat(t[2_idx], stdx::make_tuple(arg)));
    }
}
} // namespace detail

template <typename Fmt, typename... Args>
constexpr auto format(Fmt, Args... args) {
    using namespace stdx::literals;
    auto t = stdx::make_tuple(args...);
    auto r =
        t.fold_left(stdx::make_tuple(""_sc, Fmt{}, stdx::tuple{}),
                    [](auto x, auto y) { return detail::process_arg(x, y); });
    if constexpr (r[2_idx].size() == 0) {
        return r[0_idx] + r[1_idx];
    } else {
        return lazy_string_format{r[0_idx] + r[1_idx], r[2_idx]};
    }
}

template <typename T> struct formatter {
    constexpr explicit formatter(T) {}

    template <typename... Ts> constexpr auto operator()(Ts &&...args) {
        return format(T{}, std::forward<Ts>(args)...);
    }
};
} // namespace sc
