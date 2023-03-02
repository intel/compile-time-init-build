#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <interrupt/manager_interface.hpp>

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace interrupt {

/**
 * Created by calling Manager.build().
 *
 * Manager::impl is the runtime component of Manager. It is responsible for
 * initializing and running interrupts while using the least amount of run time,
 * instruction memory, and data memory. It will only initialize interrupts that
 * have interrupt service routines associated with them. If any irq is unused,
 * it will not even generate any code for the unused irqs.
 *
 * @tparam IrqImplTypes
 *      irq and shared_irq implementations. These are created by calling build()
 * on each of the irq and shared_irq instances from within Manager.
 */
template <typename InterruptHal, typename Dynamic, typename... IrqImplTypes>
class manager_impl : public manager_interface {
  private:
    cib::tuple<IrqImplTypes...> irq_impls;

    template <std::size_t Key, typename Value> struct irq_pair {};
    template <typename... Ts> struct irq_map : Ts... {};

    template <std::size_t K, typename Default>
    constexpr static auto lookup(...) -> Default;
    template <std::size_t K, typename Default, typename V>
    constexpr static auto lookup(irq_pair<K, V>) -> V;

  public:
    explicit constexpr manager_impl(IrqImplTypes... impls)
        : irq_impls{impls...} {}

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    void init() const final {
        // TODO: log exact interrupt manager configuration
        //       (should be a single compile-time string with no arguments)
        init_mcu_interrupts();
        init_sub_interrupts();
    }

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    void init_mcu_interrupts() const final {
        InterruptHal::init();
        cib::for_each(
            [](auto irq) { irq.template init_mcu_interrupts<InterruptHal>(); },
            irq_impls);
    }

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    void init_sub_interrupts() const final {
        auto const interrupt_enables_tuple =
            irq_impls.apply([](auto... irqs_pack) {
                return cib::tuple_cat(irqs_pack.get_interrupt_enables()...);
            });

        interrupt_enables_tuple.apply([]<typename... Enables>(Enables...) {
            Dynamic::template enable_by_field<true, Enables...>();
        });
    }

    /**
     * Execute the given IRQ number.
     *
     * The microcontroller's interrupt vector table should be configured to call
     * this method for each IRQ it supports.
     *
     * @tparam IrqNumber
     *      The IRQ number that has been triggered by hardware.
     */
    template <std::size_t IrqNumber> inline void run() const {
        using M = irq_map<irq_pair<IrqImplTypes::irq_number, IrqImplTypes>...>;
        using irq_t = decltype(lookup<IrqNumber, void>(std::declval<M>()));

        if constexpr (not std::is_void_v<irq_t>) {
            cib::get<irq_t>(irq_impls).template run<InterruptHal>();
        }
    }

    /**
     * @return The highest active IRQ number.
     */
    [[nodiscard]] constexpr auto max_irq() const -> std::size_t {
        return std::max({IrqImplTypes::irq_number...});
    }
};
} // namespace interrupt
