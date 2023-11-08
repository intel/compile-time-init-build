#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
/**
 * Declare a shared interrupt with one or more SubIrqs.
 *
 * A shared interrupt declares one hardware irq that may be caused by one or
 * more different sub-interrupts. When a shared_irq is triggered, it will
 * determine which sub_irq needs to be triggered.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of shared_irq.
 *
 * @tparam IrqNumber
 *      Hardware IRQ number.
 *
 * @tparam IrqPriority
 *      Hardware IRQ priority.
 *
 * @tparam SubIrqs
 *      One or more sub_irq types in this shared_irq.
 */
template <irq_num_t IrqNumber, std::size_t IrqPriority, typename Policies,
          typename... SubIrqs>
struct shared_irq {
  public:
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

    using irq_callback_t = void;
    constexpr static auto children = stdx::tuple<SubIrqs...>{};
    constexpr static auto irq_number = IrqNumber;
};
} // namespace interrupt
