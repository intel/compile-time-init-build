#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <concepts>

namespace interrupt {
namespace detail {
template <typename T, template <typename...> typename X>
concept specializes = stdx::is_specialization_of_v<std::remove_cvref_t<T>, X>;
}

template <typename T>
concept root_config = requires {
    { T::children } -> detail::specializes<stdx::tuple>;
    { T::descendants } -> detail::specializes<stdx::tuple>;
    typename T::template dynamic_controller_t<int>;
};

template <typename T>
concept base_irq_config =
    status_policy<typename T::status_policy_t> and
    detail::specializes<typename T::resources_t, resource_list> and requires {
        { T::template enable<true>() } -> std::same_as<void>;
        { T::children } -> detail::specializes<stdx::tuple>;
        { T::descendants } -> detail::specializes<stdx::tuple>;
    };

template <typename T>
concept irq_config = base_irq_config<T> and requires {
    { T::irq_number } -> std::same_as<irq_num_t const &>;
};

template <typename T>
concept sub_irq_config = base_irq_config<T> and requires {
    T::enable_field;
    T::status_field;
};

template <typename T>
concept base_irq_interface = requires(T const &t) {
    { t.get_interrupt_enables() } -> detail::specializes<stdx::tuple>;
    { t.run() } -> std::same_as<void>;
};

template <typename T>
concept irq_interface = base_irq_interface<T> and requires(T const &t) {
    { T::irq_number } -> std::same_as<irq_num_t const &>;
    { t.init_mcu_interrupts() } -> std::same_as<void>;
};

template <typename T>
concept sub_irq_interface = base_irq_interface<T>;

template <typename T, typename Flow>
concept nexus_for = requires {
    T::template service<Flow>();
    { T::template service<Flow>.active } -> std::same_as<bool const &>;
};
} // namespace interrupt
