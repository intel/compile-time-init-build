#include <interrupt/config/fwd.hpp>
#include <interrupt/fwd.hpp>

#ifndef CIB_INTERRUPT_IMPL_SHARED_SUB_IRQ_IMPL_HPP
#define CIB_INTERRUPT_IMPL_SHARED_SUB_IRQ_IMPL_HPP

namespace interrupt {
template <typename ConfigT, typename... SubIrqImpls>
struct shared_sub_irq_impl {
  public:
    /**
     * True if this shared_irq::impl has any active sub_irq::Impls, otherwise
     * False.
     *
     * This is used to optimize compiled size and runtime performance. Unused
     * irqs should not consume any resources.
     */
    static bool constexpr active = (SubIrqImpls::active || ... || false);

  private:
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action =
        ConfigT::template enable_action<InterruptHal, en>;

    constexpr static auto enable_field = ConfigT::enable_field;
    constexpr static auto status_field = ConfigT::status_field;
    using StatusPolicy = typename ConfigT::StatusPolicy;

    hana::tuple<SubIrqImpls...> sub_irq_impls;

  public:
    explicit constexpr shared_sub_irq_impl(SubIrqImpls const &...impls)
        : sub_irq_impls(impls...) {}

    auto get_interrupt_enables() const {
        if constexpr (active) {
            auto const active_sub_irq_impls =
                hana::filter(sub_irq_impls, [](auto irq) {
                    return hana::bool_c<decltype(irq)::active>;
                });

            auto const sub_irq_interrupt_enables =
                hana::unpack(active_sub_irq_impls, [](auto &&...irqs) {
                    return hana::flatten(
                        hana::make_tuple(irqs.get_interrupt_enables()...));
                });

            return hana::append(sub_irq_interrupt_enables, *enable_field);

        } else {
            return hana::make_tuple();
        }
    }

    /**
     * Evaluate interrupt status of each sub_irq::impl and run each one with a
     * pending interrupt. Clear any hardware interrupt pending bits as
     * necessary.
     */
    inline void run() const {
        if constexpr (active) {
            if (apply(read((*enable_field)(1))) &&
                apply(read((*status_field)(1)))) {
                StatusPolicy::run([&] { apply(clear(*status_field)); },
                                  [&] {
                                      hana::for_each(
                                          sub_irq_impls,
                                          [](auto irq) { irq.run(); });
                                  });
            }
        }
    }
};
} // namespace interrupt

#endif // CIB_INTERRUPT_IMPL_SHARED_SUB_IRQ_IMPL_HPP
