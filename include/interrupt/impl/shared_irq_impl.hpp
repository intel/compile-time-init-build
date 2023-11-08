#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>

namespace interrupt {
template <typename Config, typename... SubIrqImpls> struct shared_irq_impl {
  public:
    template <bool Enable>
    constexpr static FunctionPtr enable_action =
        Config::template enable_action<Enable>;
    using status_policy_t = typename Config::status_policy_t;

    constexpr static auto irq_number = Config::irq_number;

    /**
     * True if this shared_irq::impl has any active sub_irq::Impls, otherwise
     * False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * irqs should not consume any resources.
     */
    constexpr static bool active = (SubIrqImpls::active or ...);

  private:
    stdx::tuple<SubIrqImpls...> sub_irq_impls;

    template <typename Irq> using is_active = std::bool_constant<Irq::active>;

  public:
    explicit constexpr shared_irq_impl(SubIrqImpls const &...impls)
        : sub_irq_impls{impls...} {}

    /**
     * Initialize and enable the hardware interrupt along with
     *
     * This should be used only by interrupt::Manager.
     *
     */
    inline void init_mcu_interrupts() const {
        // initialize the main irq hardware
        // TODO: unit test says MCU interrupts always get enabled for shared
        // interrupts...why??
        enable_action<true>();
    }

    auto get_interrupt_enables() const {
        if constexpr (active) {
            auto const active_sub_irq_impls =
                stdx::filter<is_active>(sub_irq_impls);

            return active_sub_irq_impls.apply([](auto &&...irqs) {
                return stdx::tuple_cat(irqs.get_interrupt_enables()...);
            });
        } else {
            return stdx::tuple{};
        }
    }

    /**
     * Evaluate interrupt status of each sub_irq::impl and run each one with a
     * pending interrupt. Clear any hardware interrupt pending bits as
     * necessary.
     *
     * This should be used only by interrupt::Manager.
     *
     */
    inline void run() const {
        if constexpr (active) {
            hal::run<status_policy_t>(irq_number, [&] {
                stdx::for_each([](auto irq) { irq.run(); }, sub_irq_impls);
            });
        }
    }
};
} // namespace interrupt
