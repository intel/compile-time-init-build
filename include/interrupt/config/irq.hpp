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
 * @tparam IrqNumberT
 *      Hardware IRQ number.
 *
 * @tparam IrqPriorityT
 *      Hardware IRQ priority.
 */
template <std::size_t IrqNumberT, std::size_t IrqPriorityT,
          typename IrqCallbackT, typename PoliciesT>
struct irq {
    template <bool en>
    constexpr static FunctionPtr enable_action =
        hal::irq_init<en, IrqNumberT, IrqPriorityT>;
    using StatusPolicy = typename PoliciesT::template type<status_clear_policy,
                                                           clear_status_first>;
    constexpr static auto resources =
        PoliciesT::template get<required_resources_policy,
                                required_resources<>>()
            .resources;
    using IrqCallbackType = IrqCallbackT;
    constexpr static stdx::tuple<> children{};

    constexpr static auto irq_number = IrqNumberT;
};
} // namespace interrupt
