#pragma once

#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/extend.hpp>

#include <stdx/compiler.hpp>
#include <stdx/concepts.hpp>
#include <stdx/ct_format.hpp>
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

    CONSTEVAL explicit runtime_conditional(Configs const &...configs)
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
                        return extend<typename E::service_type, Args...>{
                            Args{}...};
                    },
                    args_tuple);
            },
            body.extends_tuple());
    }

    [[nodiscard]] constexpr auto exports_tuple() const {
        return body.exports_tuple();
    }
};

template <typename T>
concept runtime_predicate =
    std::default_initializable<T> and stdx::predicate<T>;

template <stdx::ct_string Name, runtime_predicate... Ps>
struct runtime_condition {
    constexpr static auto predicates = stdx::make_tuple(Ps{}...);

    constexpr static auto ct_name = Name;

    template <typename... Configs>
    [[nodiscard]] CONSTEVAL auto operator()(Configs const &...configs) const {
        return detail::runtime_conditional<runtime_condition<Name, Ps...>,
                                           Configs...>{configs...};
    }

    explicit operator bool() const { return (Ps{}() and ...); }
};

template <stdx::ct_string LhsName, typename... LhsPs, stdx::ct_string RhsName,
          typename... RhsPs>
[[nodiscard]] constexpr auto
operator and(runtime_condition<LhsName, LhsPs...> const &lhs,
             runtime_condition<RhsName, RhsPs...> const &rhs) {
    if constexpr ((sizeof...(LhsPs) + sizeof...(RhsPs)) == 0) {
        return runtime_condition<"always">{};

    } else if constexpr (sizeof...(LhsPs) == 0) {
        return rhs;

    } else if constexpr (sizeof...(RhsPs) == 0) {
        return lhs;

    } else {
        constexpr auto name =
            stdx::ct_format<"{} and {}">(CX_VALUE(LhsName), CX_VALUE(RhsName));

        return runtime_condition<name, LhsPs..., RhsPs...>{};
    }
}

using always_condition_t = runtime_condition<"always">;
constexpr auto always_condition = always_condition_t{};
} // namespace cib::detail
