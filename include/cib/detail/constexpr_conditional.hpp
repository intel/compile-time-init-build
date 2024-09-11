#pragma once

#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/extend.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace cib::detail {
template <typename Cond, typename... Configs>
struct constexpr_conditional : config_item {
    detail::config<Configs...> body;

    CONSTEVAL explicit constexpr_conditional(Configs const &...configs)
        : body{configs...} {}

    [[nodiscard]] constexpr auto extends_tuple() const {
        if constexpr (Cond{}) {
            return body.extends_tuple();
        } else {
            return stdx::tuple<>{};
        }
    }

    [[nodiscard]] constexpr auto exports_tuple() const {
        if constexpr (Cond{}) {
            return body.exports_tuple();
        } else {
            return stdx::tuple<>{};
        }
    }
};

template <stdx::ct_string Name, typename... Ps> struct constexpr_condition {
    constexpr static auto predicates = stdx::make_tuple(Ps{}...);

    constexpr static auto ct_name = Name;

    template <typename... Configs>
    [[nodiscard]] CONSTEVAL auto operator()(Configs const &...configs) const {
        return detail::constexpr_conditional<constexpr_condition<Name, Ps...>,
                                             Configs...>{configs...};
    }

    explicit constexpr operator bool() const { return (Ps{}() and ...); }
};

} // namespace cib::detail
