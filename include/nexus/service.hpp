#pragma once

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>
#include <stdx/utility.hpp>

#include <concepts>
#include <memory>
#include <type_traits>

namespace cib {
namespace archetypes {
template <typename T> struct initialized_service {
    constexpr static auto value = T{};
};
struct nexus {};
} // namespace archetypes

template <typename Interface, typename T>
constexpr auto to_interface(T const &t) {
    if constexpr (std::is_convertible_v<T, Interface>) {
        return t;
    } else {
        if constexpr (std::is_pointer_v<T>) {
            return t;
        } else {
            return std::addressof(t);
        }
    }
}

template <typename Built, typename Interface>
concept interface_convertible =
    requires(Built const &b) { to_interface<Interface>(b); };

namespace detail {
struct add_base {
    constexpr auto add() const -> void;
};
template <typename T> struct add_tester : T, add_base {};

template <typename T>
concept has_add_function = not requires(add_tester<T> &t) { t.add(); };

template <typename Builder, typename Meta>
concept injected_builder_for = requires(Builder &b) {
    {
        Builder::template build<archetypes::initialized_service<Builder>,
                                archetypes::nexus>()
    } -> interface_convertible<typename Meta::interface_t>;
};
} // namespace detail

template <typename Builder, typename Meta>
concept builder_for = detail::has_add_function<Builder> and
                      detail::injected_builder_for<Builder, Meta>;

template <typename T>
concept builder_meta = builder_for<typename T::builder_t, T> and requires {
    { T::uninitialized() } -> std::same_as<typename T::interface_t>;
};

template <builder_meta T> using builder_t = typename T::builder_t;
template <builder_meta T> using interface_t = typename T::interface_t;

template <builder_meta ServiceMeta>
constinit inline auto service = ServiceMeta::uninitialized();

template <typename R> using by_name_service_t = auto (*)() -> R;

template <stdx::ct_string Name, typename R>
constexpr inline by_name_service_t<R> undefined_by_name_service_v = []() -> R {
    constexpr auto msg = STDX_CT_FORMAT("Invoking service ({} :: () -> {}) by "
                                        "name before it is initialized",
                                        Name, R);
    stdx::panic<msg>();
    stdx::unreachable();
};

template <stdx::ct_string Name, typename R = void>
constinit inline by_name_service_t<R> invoke_service =
    undefined_by_name_service_v<Name, R>;

} // namespace cib
