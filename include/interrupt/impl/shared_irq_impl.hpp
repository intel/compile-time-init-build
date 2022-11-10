#pragma once

#include <interrupt/config/fwd.hpp>

#include <boost/hana.hpp>

namespace interrupt {
template <typename ConfigT, typename... SubIrqImpls> struct shared_irq_impl {
  public:
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action =
        ConfigT::template enable_action<InterruptHal, en>;
    using StatusPolicy = typename ConfigT::StatusPolicy;

    constexpr static auto irq_number = ConfigT::irq_number;

    /**
     * True if this shared_irq::impl has any active sub_irq::Impls, otherwise
     * False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * irqs should not consume any resources.
     */
    static bool constexpr active = (SubIrqImpls::active || ... || false);

  private:
    boost::hana::tuple<SubIrqImpls...> sub_irq_impls;

  public:
    explicit constexpr shared_irq_impl(SubIrqImpls const &...impls)
        : sub_irq_impls(impls...) {}

    /**
     * Initialize and enable the hardware interrupt along with
     *
     * This should be used only by interrupt::Manager.
     *
     * @tparam InterruptHal
     *      The hardware abstraction layer that knows how to initialize hardware
     * interrupts.
     */
    template <typename InterruptHal> inline void init_mcu_interrupts() const {
        // initialize the main irq hardware
        // TODO: unit test says MCU interrupts always get enabled for shared
        // interrupts...why??
        enable_action<InterruptHal, true>();
    }

    auto get_interrupt_enables() const {
        using namespace boost;
        if constexpr (active) {
            auto const active_sub_irq_impls =
                hana::filter(sub_irq_impls, [](auto irq) {
                    return hana::bool_c<decltype(irq)::active>;
                });

            return hana::unpack(active_sub_irq_impls, [](auto &&...irqs) {
                return hana::flatten(
                    hana::make_tuple(irqs.get_interrupt_enables()...));
            });

        } else {
            return hana::make_tuple();
        }
    }

    /**
     * Evaluate interrupt status of each sub_irq::impl and run each one with a
     * pending interrupt. Clear any hardware interrupt pending bits as
     * necessary.
     *
     * This should be used only by interrupt::Manager.
     *
     * @tparam InterruptHal
     *      The hardware abstraction layer that knows how to clear pending
     * interrupt status.
     */
    template <typename InterruptHal> inline void run() const {
        if constexpr (active) {
            InterruptHal::template run<StatusPolicy>(irq_number, [&] {
                boost::hana::for_each(sub_irq_impls,
                                      [](auto irq) { irq.run(); });
            });
        }
    }
};
} // namespace interrupt
