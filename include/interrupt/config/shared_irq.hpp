#pragma once

#include <cib/tuple.hpp>
#include <interrupt/config/fwd.hpp>
#include <interrupt/policies.hpp>

#include <boost/hana.hpp>

namespace interrupt {
namespace hana = boost::hana;

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
 * @tparam IrqNumberT
 *      Hardware IRQ number.
 *
 * @tparam IrqPriorityT
 *      Hardware IRQ priority.
 *
 * @tparam SubIrqs
 *      One or more sub_irq types in this shared_irq.
 */
template <std::size_t IrqNumberT, std::size_t IrqPriorityT, typename PoliciesT,
          typename... SubIrqs>
struct shared_irq {
  public:
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action =
        InterruptHal::template irqInit<en, IrqNumberT, IrqPriorityT>;
    constexpr static auto enable_field = hana::nothing;
    constexpr static auto status_field = hana::nothing;
    using StatusPolicy = typename PoliciesT::template type<status_clear_policy,
                                                           clear_status_first>;
    constexpr static auto resources =
        PoliciesT::template get<required_resources_policy,
                                required_resources<>>()
            .resources;
    using IrqCallbackType = void;
    constexpr static cib::tuple<SubIrqs...> children{};

    constexpr static auto irq_number = IrqNumberT;
};
} // namespace interrupt
