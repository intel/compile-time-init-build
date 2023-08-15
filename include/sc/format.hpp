#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <sc/detail/conversions.hpp>
#include <sc/fwd.hpp>
#include <sc/lazy_string_format.hpp>
#include <sc/string_constant.hpp>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace sc {
namespace detail {
template <typename T>
concept compile_time_field = requires { T::value; };

template <compile_time_field T>
[[nodiscard]] CIB_CONSTEVAL auto field_value(T) {
    if constexpr (std::is_enum_v<decltype(T::value)>) {
        return detail::enum_as_string<T::value>();
    } else {
        return T::value;
    }
}

template <typename T>
[[nodiscard]] CIB_CONSTEVAL auto field_value(sc::type_name<T>) {
    return detail::type_as_string<T>();
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
constexpr auto process_arg(cib::tuple<Str, Fmt, RuntimeTuple> t, Arg arg) {
    using namespace cib::tuple_literals;
    constexpr auto p = split_format_spec<Fmt>();
    if constexpr (requires { field_value(arg); }) {
        return cib::make_tuple(t[0_idx] + format1(p.first, arg), p.second,
                               t[2_idx]);
    } else if constexpr (requires { arg.args; }) {
        return cib::make_tuple(t[0_idx] + format1(p.first, arg.str), p.second,
                               cib::tuple_cat(t[2_idx], arg.args));
    } else {
        return cib::make_tuple(t[0_idx] + p.first, p.second,
                               cib::tuple_cat(t[2_idx], cib::make_tuple(arg)));
    }
}
} // namespace detail

template <typename Fmt, typename... Args>
constexpr auto format(Fmt, Args... args) {
    using namespace cib::tuple_literals;
    auto t = cib::make_tuple(args...);
    auto r =
        t.fold_left(cib::make_tuple(""_sc, Fmt{}, cib::tuple{}),
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
