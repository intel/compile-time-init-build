#pragma once

#include <interrupt/fwd.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>

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
    constexpr static bool active = (SubIrqImpls::active or ...);

  private:
    template <bool en>
    constexpr static FunctionPtr enable_action =
        ConfigT::template enable_action<en>;

    constexpr static auto enable_field = ConfigT::enable_field;
    constexpr static auto status_field = ConfigT::status_field;
    using StatusPolicy = typename ConfigT::StatusPolicy;

    stdx::tuple<SubIrqImpls...> sub_irq_impls;

    template <typename Irq> using is_active = std::bool_constant<Irq::active>;

  public:
    explicit constexpr shared_sub_irq_impl(SubIrqImpls const &...impls)
        : sub_irq_impls{impls...} {}

    auto get_interrupt_enables() const {
        if constexpr (active) {
            auto const active_sub_irq_impls =
                stdx::filter<is_active>(sub_irq_impls);

            return active_sub_irq_impls.apply([&](auto &&...irqs) {
                return stdx::tuple_cat(irqs.get_interrupt_enables()...,
                                       stdx::make_tuple(enable_field));
            });

        } else {
            return stdx::make_tuple();
        }
    }

    /**
     * Evaluate interrupt status of each sub_irq::impl and run each one with a
     * pending interrupt. Clear any hardware interrupt pending bits as
     * necessary.
     */
    inline void run() const {
        if constexpr (active) {
            if (apply(read(enable_field(1))) && apply(read(status_field(1)))) {
                StatusPolicy::run([&] { apply(clear(status_field)); },
                                  [&] {
                                      stdx::for_each(
                                          [](auto irq) { irq.run(); },
                                          sub_irq_impls);
                                  });
            }
        }
    }
};
} // namespace interrupt
