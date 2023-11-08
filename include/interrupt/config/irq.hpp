#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>

#include <cstddef>

namespace interrupt {
/**
 * Declare a simple unshared interrupt.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of irq.
 *
 * @tparam IrqNumber
 *      Hardware IRQ number.
 *
 * @tparam IrqPriority
 *      Hardware IRQ priority.
 */
template <irq_num_t IrqNumber, std::size_t IrqPriority, typename IrqCallback,
          typename Policies>
struct irq {
    template <bool Enable>
    constexpr static FunctionPtr enable_action =
        hal::irq_init<Enable, IrqNumber, IrqPriority>;
    using status_policy_t =
        typename Policies::template type<status_clear_policy,
                                         clear_status_first>;
    constexpr static auto resources =
        Policies::template get<required_resources_policy,
                               required_resources<>>()
            .resources;
    using irq_callback_t = IrqCallback;

    constexpr static auto children = stdx::tuple{};
    constexpr static auto irq_number = IrqNumber;
};
} // namespace interrupt
