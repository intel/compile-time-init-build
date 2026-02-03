#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/ct_conversions.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <concepts>
#include <cstddef>

namespace interrupt {
namespace detail {
template <typename T, template <typename...> typename X>
concept specializes = stdx::is_specialization_of_v<std::remove_cvref_t<T>, X>;
}

namespace archetypes {
struct hal {
    static auto init() -> void;

    template <bool Enable, irq_num_t IrqNumber, std::size_t Priority>
    static auto irq_init() -> void;

    template <status_policy P>
    static auto run(irq_num_t, stdx::invocable auto const &isr) -> void;

    template <typename Field> static auto get_register() -> Field;
    template <typename Register> using register_datatype_t = int;
    template <typename Register, typename Field> constexpr static auto mask = 0;
    template <typename Register> static auto write(auto) -> void;
};
} // namespace archetypes

template <typename T>
concept root_config = requires {
    { T::children } -> detail::specializes<stdx::tuple>;
    { T::descendants } -> detail::specializes<stdx::tuple>;
    typename T::template dynamic_controller_t<T, archetypes::hal>;
};

template <typename T>
concept base_irq_config =
    status_policy<typename T::status_policy_t> and
    detail::specializes<typename T::resources_t, resource_list> and requires {
        { T::template enable<true, archetypes::hal>() } -> std::same_as<void>;
        { T::children } -> detail::specializes<stdx::tuple>;
        { T::descendants } -> detail::specializes<stdx::tuple>;
    };

template <typename T>
concept irq_config = base_irq_config<T> and requires {
    { T::irq_number } -> std::same_as<irq_num_t const &>;
};

template <typename T>
concept sub_irq_config = base_irq_config<T> and requires {
    typename T::enable_field_t;
    typename T::status_field_t;
};

template <typename T>
concept base_irq_interface = requires(T const &t) {
    { t.template run<archetypes::hal>() } -> std::same_as<void>;
};

template <typename T>
concept irq_interface = base_irq_interface<T> and requires(T const &t) {
    { T::irq_number } -> std::same_as<irq_num_t const &>;
    { t.template init<archetypes::hal>() } -> std::same_as<void>;
};

template <typename T>
concept sub_irq_interface = base_irq_interface<T>;

template <typename T, typename Flow>
concept nexus_for = requires {
    T::template service<Flow>();
    { T::template service<Flow>.active } -> std::same_as<bool const &>;
};

namespace detail {
template <typename T> constexpr auto config_string1() {
    if constexpr (requires {
                      []<auto N>(stdx::ct_string<N>) {}(T::config());
                  }) {
        return stdx::ct<T::config()>();
    } else if constexpr (requires {
                             []<auto N>(stdx::ct_string<N>) {}(T::name);
                         }) {
        return stdx::ct<T::name>();
    } else {
        constexpr auto s = stdx::type_as_string<T>();
        return stdx::ct<stdx::ct_string<s.size() + 1>{s}>();
    }
}

template <typename... Ts> constexpr auto config_string_for() {
    using namespace stdx::literals;
    return stdx::tuple{config_string1<Ts>()...}.join(
        ""_ctst, [](auto lhs, auto rhs) { return lhs + ", "_ctst + rhs; });
}
} // namespace detail

struct no_field_t;

template <typename Field>
constexpr inline auto is_no_field_v = std::same_as<Field, no_field_t>;
template <typename Field>
using is_no_field = std::bool_constant<is_no_field_v<Field>>;
} // namespace interrupt
