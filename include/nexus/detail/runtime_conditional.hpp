#pragma once

#include <nexus/detail/config_details.hpp>
#include <nexus/detail/config_item.hpp>
#include <nexus/extend.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace cib::detail {
namespace poison {
template <typename... Ts>
constexpr auto make_runtime_conditional(Ts &&...) = delete;
}

template <typename Cond, typename... Configs>
struct runtime_conditional : config_item {
    detail::config<Configs...> body;

    consteval explicit runtime_conditional(Configs const &...configs)
        : body{configs...} {}

    [[nodiscard]] constexpr auto extends_tuple() const {
        return stdx::transform(
            []<typename E>(E e) {
                auto args_tuple = stdx::transform(
                    []<typename Arg>(Arg) {
                        using poison::make_runtime_conditional;
                        return make_runtime_conditional(Cond{}, Arg{});
                    },
                    e.args_tuple);

                return stdx::apply(
                    []<typename... Args>(Args...) {
                        if constexpr (requires { typename E::service_type; }) {
                            return extend<typename E::service_type>(Args{}...);
                        } else {
                            return extend<E::name>(Args{}...);
                        }
                    },
                    args_tuple);
            },
            body.extends_tuple());
    }

    [[nodiscard]] constexpr static auto get_exports()
        -> decltype(detail::config<Configs...>::get_exports()) {
        return {};
    }
};

template <typename T>
concept runtime_predicate =
    std::default_initializable<T> and stdx::predicate<T>;

namespace s {
template <auto Name, runtime_predicate... Ps>
struct runtime_condition : stdx::with_shrunk_name<Name> {
    constexpr static auto predicates = stdx::make_tuple(Ps{}...);

    template <typename... Configs>
    [[nodiscard]] consteval auto operator()(Configs const &...configs) const {
        return detail::runtime_conditional<runtime_condition<Name, Ps...>,
                                           Configs...>{configs...};
    }

    explicit operator bool() const { return (Ps{}() and ...); }
};

template <auto LhsName, typename... LhsPs, auto RhsName, typename... RhsPs>
[[nodiscard]] constexpr auto
operator and(runtime_condition<LhsName, LhsPs...> const &lhs,
             runtime_condition<RhsName, RhsPs...> const &rhs) {
    if constexpr ((sizeof...(LhsPs) + sizeof...(RhsPs)) == 0) {
        return runtime_condition<stdx::shrink<"always">()>{};

    } else if constexpr (sizeof...(LhsPs) == 0) {
        return rhs;

    } else if constexpr (sizeof...(RhsPs) == 0) {
        return lhs;

    } else {
        constexpr auto name = stdx::ct_format<"{} and {}">(stdx::ct<LhsName>(),
                                                           stdx::ct<RhsName>());
        return runtime_condition<stdx::shrink<name>(), LhsPs..., RhsPs...>{};
    }
}
} // namespace s

template <stdx::ct_string Name, runtime_predicate... Ps>
using runtime_condition = s::runtime_condition<stdx::shrink<Name>(), Ps...>;

using always_condition_t = runtime_condition<"always">;
constexpr auto always_condition = always_condition_t{};
} // namespace cib::detail
