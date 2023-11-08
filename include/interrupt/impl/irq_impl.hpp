#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
/**
 * Runtime implementation of the irq.
 *
 * @tparam Flow
 *      The actual flow::impl<Size> type that can contain all of the interrupt
 * service routines from the irq's flow::Builder<>. This needs to be accurately
 * sized to ensure it can indicate whether any interrupt service routines are
 * registered.
 */
template <typename Config, typename Flow> struct irq_impl {
  public:
    template <bool Enable>
    constexpr static FunctionPtr enable_action =
        Config::template enable_action<Enable>;
    using status_policy_t = typename Config::status_policy_t;

    constexpr static auto irq_number = Config::irq_number;

    /**
     * True if this irq::impl has any interrupt service routines to execute,
     * otherwise False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * Irqs should not consume any resources.
     */
    constexpr static bool active = Flow::active;

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
     */
    inline void init_mcu_interrupts() const { enable_action<active>(); }

    [[nodiscard]] inline auto get_interrupt_enables() const -> stdx::tuple<> {
        return {};
    }

    /**
     * Run the interrupt service routine and clear any pending interrupt status.
     *
     * This should be used only by interrupt::Manager.
     *
     */
    inline void run() const {
        if constexpr (active) {
            hal::run<status_policy_t>(irq_number,
                                      [&]() { interrupt_service_routine(); });
        }
    }
};
} // namespace interrupt
