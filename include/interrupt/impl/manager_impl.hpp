#include <interrupt/manager_interface.hpp>

#include <boost/hana.hpp>

#include <type_traits>

#ifndef CIB_INTERRUPT_IMPL_MANAGER_IMPL_HPP
#define CIB_INTERRUPT_IMPL_MANAGER_IMPL_HPP

namespace interrupt {
namespace hana = boost::hana;
using namespace hana::literals;

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
    hana::tuple<IrqImplTypes...> irq_impls;

  public:
    explicit constexpr manager_impl(IrqImplTypes... impls)
        : irq_impls{impls...} {}

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    virtual void init() const override final {
        // TODO: log exact interrupt manager configuration
        //       (should be a single compile-time string with no arguments)
        init_mcu_interrupts();
        init_sub_interrupts();
    }

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    virtual void init_mcu_interrupts() const override final {
        InterruptHal::init();
        hana::for_each(irq_impls, [](auto irq) {
            irq.template init_mcu_interrupts<InterruptHal>();
        });
    }

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    virtual void init_sub_interrupts() const override final {
        auto const interrupt_enables_tuple =
            hana::unpack(irq_impls, [](auto... irqs_pack) {
                return hana::flatten(
                    hana::make_tuple(irqs_pack.get_interrupt_enables()...));
            });

        hana::unpack(interrupt_enables_tuple, [](auto... interrupt_enables) {
            Dynamic::template enable_by_field<true,
                                              decltype(interrupt_enables)...>();
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
        // find the IRQ with the matching number
        auto const matching_irq = hana::find_if(irq_impls, [](auto i) {
            return hana::bool_c<IrqNumber == decltype(i)::irq_number>;
        });

        auto constexpr run_irq = [](auto &irq) {
            irq.template run<InterruptHal>();
            return true;
        };

        // if the IRQ was found, then run it, otherwise do nothing
        hana::maybe(false, run_irq, matching_irq);
    }

    /**
     * @return The highest active IRQ number.
     */
    [[nodiscard]] std::size_t constexpr max_irq() const {
        auto const irq_numbers = hana::transform(
            irq_impls, [](auto irq) { return decltype(irq)::irq_number; });

        return hana::maximum(irq_numbers);
    }
};
} // namespace interrupt

#endif // CIB_INTERRUPT_IMPL_MANAGER_IMPL_HPP
