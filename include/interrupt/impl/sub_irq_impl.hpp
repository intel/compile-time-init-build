#pragma once

#include <interrupt/fwd.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
/**
 * Runtime implementation of the sub_irq.
 *
 * @tparam FlowTypeT
 *      The actual flow::impl<Size> type that can contain all of the interrupt
 * service routines from the sub_irq's flow::Builder<>. This needs to be
 * accurately sized to ensure it can indicate whether any interrupt service
 * routines are registered.
 */
template <typename ConfigT, typename FlowTypeT> struct sub_irq_impl {
  public:
    /**
     * True if this sub_irq::impl has any interrupt service routines to execute,
     * otherwise False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * SubIrqs should not consume any resources.
     */
    constexpr static bool active = FlowTypeT::active;

  private:
    constexpr static auto enable_field = ConfigT::enable_field;
    constexpr static auto status_field = ConfigT::status_field;
    using StatusPolicy = typename ConfigT::StatusPolicy;

    FunctionPtr interrupt_service_routine;

  public:
    explicit constexpr sub_irq_impl(FunctionPtr const &flow)
        : interrupt_service_routine(flow) {}

    [[nodiscard]] auto get_interrupt_enables() const {
        return stdx::make_tuple(enable_field);
    }

    /**
     * Run the interrupt service routine and clear any pending interrupt status.
     * This includes checking and clearing the interrupt status register field.
     *
     * This should be used only by interrupt::Manager.
     *
     */
    inline void run() const {
        if constexpr (active) {
            if (apply(read(enable_field)) && apply(read(status_field))) {
                StatusPolicy::run([&] { apply(clear(status_field)); },
                                  [&] { interrupt_service_routine(); });
            }
        }
    }
};
} // namespace interrupt
