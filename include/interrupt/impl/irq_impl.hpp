#pragma once

#include <cib/tuple.hpp>
#include <interrupt/config/fwd.hpp>
#include <interrupt/fwd.hpp>

namespace interrupt {
/**
 * Runtime implementation of the irq.
 *
 * @tparam FlowTypeT
 *      The actual flow::impl<Size> type that can contain all of the interrupt
 * service routines from the irq's flow::Builder<>. This needs to be accurately
 * sized to ensure it can indicate whether any interrupt service routines are
 * registered.
 */
template <typename ConfigT, typename FlowTypeT> struct irq_impl {
  public:
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action =
        ConfigT::template enable_action<InterruptHal, en>;
    using StatusPolicy = typename ConfigT::StatusPolicy;

    constexpr static auto irq_number = ConfigT::irq_number;

    /**
     * True if this irq::impl has any interrupt service routines to execute,
     * otherwise False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * Irqs should not consume any resources.
     */
    static bool constexpr active = FlowTypeT::active;

  private:
    FunctionPtr interrupt_service_routine;

  public:
    explicit constexpr irq_impl(FunctionPtr const &flow)
        : interrupt_service_routine(flow) {}

    /**
     * Initialize and enable the hardware interrupt.
     *
     * This should be used only by interrupt::Manager.
     *
     * @tparam InterruptHal
     *      The hardware abstraction layer that knows how to initialize hardware
     * interrupts.
     */
    template <typename InterruptHal> inline void init_mcu_interrupts() const {
        enable_action<InterruptHal, active>();
    }

    [[nodiscard]] inline auto get_interrupt_enables() const -> cib::tuple<> {
        return {};
    }

    /**
     * Run the interrupt service routine and clear any pending interrupt status.
     *
     * This should be used only by interrupt::Manager.
     *
     * @tparam InterruptHal
     *      The hardware abstraction layer that knows how to clear pending
     * interrupt status.
     */
    template <typename InterruptHal> inline void run() const {
        if constexpr (active) {
            InterruptHal::template run<StatusPolicy>(
                irq_number, [&]() { interrupt_service_routine(); });
        }
    }
};
} // namespace interrupt
