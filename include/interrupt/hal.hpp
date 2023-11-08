#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/concepts.hpp>
#include <stdx/type_traits.hpp>

namespace interrupt {
template <typename T>
concept hal_interface = requires(T const &t, void (*isr)()) {
    { t.init() } -> stdx::same_as<void>;
    {
        t.template irq_init<true, irq_num_t{}, std::size_t{}>()
    } -> stdx::same_as<void>;
    {
        t.template run<dont_clear_status>(irq_num_t{}, isr)
    } -> stdx::same_as<void>;
};

template <typename...> struct null_hal {
    static auto init() -> void { undefined(); }

    template <bool Enable, irq_num_t IrqNumber, std::size_t PriorityLevel>
    static auto irq_init() -> void {
        undefined();
    }

    template <status_policy>
    static auto run(irq_num_t, stdx::invocable auto const &) -> void {
        undefined();
    }

  private:
    static auto undefined() -> void {
        static_assert(stdx::always_false_v<null_hal>,
                      "No interrupt HAL defined: inject one");
    }
};
static_assert(hal_interface<null_hal<>>);

template <typename...> inline auto injected_hal = null_hal{};

struct hal {
    template <typename... Ts>
        requires(sizeof...(Ts) == 0)
    static auto init() -> void {
        injected_hal<Ts...>.init();
    }

    template <bool Enable, irq_num_t IrqNumber, int Priority, typename... Ts>
        requires(sizeof...(Ts) == 0)
    static auto irq_init() -> void {
        injected_hal<Ts...>.template irq_init<Enable, IrqNumber, Priority>();
    }

    template <status_policy P, typename... Ts>
        requires(sizeof...(Ts) == 0)
    static auto run(irq_num_t irq, stdx::invocable auto const &isr) -> void {
        injected_hal<Ts...>.template run<P>(irq, isr);
    }
};

static_assert(hal_interface<hal>);
} // namespace interrupt
