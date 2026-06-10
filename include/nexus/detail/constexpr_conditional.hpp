#pragma once

#include <nexus/detail/config_details.hpp>
#include <nexus/detail/config_item.hpp>
#include <nexus/detail/extend.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

namespace cib::detail {
template <typename Cond, typename... Configs>
struct constexpr_conditional : config_item {
    detail::config<Configs...> body;

    consteval explicit constexpr_conditional(Configs const &...configs)
        : body{configs...} {}

    [[nodiscard]] constexpr auto extends_tuple() const {
        if constexpr (Cond{}) {
            return body.extends_tuple();
        } else {
            return stdx::tuple<>{};
        }
    }

    [[nodiscard]] constexpr static auto get_exports() -> stdx::conditional_t<
        static_cast<bool>(Cond{}),
        decltype(detail::config<Configs...>::get_exports()),
        stdx::type_list<>> {
        return {};
    }
};

template <stdx::ct_string Name, typename... Ps> struct constexpr_condition {
    constexpr static auto predicates = stdx::make_tuple(Ps{}...);

    constexpr static auto ct_name = Name;

    template <typename... Configs>
    [[nodiscard]] consteval auto operator()(Configs const &...configs) const {
        return detail::constexpr_conditional<constexpr_condition<Name, Ps...>,
                                             Configs...>{configs...};
    }

    explicit constexpr operator bool() const { return (Ps{}() and ...); }
};

} // namespace cib::detail
