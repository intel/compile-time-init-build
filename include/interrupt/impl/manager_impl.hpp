#pragma once

#include <interrupt/hal.hpp>
#include <interrupt/manager_interface.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

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
 * @tparam IrqImpls
 *      irq and shared_irq implementations. These are created by calling build()
 * on each of the irq and shared_irq instances from within Manager.
 */
template <typename Dynamic, typename... IrqImpls>
class manager_impl : public manager_interface {
  private:
    stdx::tuple<IrqImpls...> irq_impls;

  public:
    explicit constexpr manager_impl(IrqImpls... impls) : irq_impls{impls...} {}

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
        hal::init();
        stdx::for_each([](auto irq) { irq.init_mcu_interrupts(); }, irq_impls);
    }

    /**
     * Initialize the interrupt hardware and each of the active irqs.
     */
    void init_sub_interrupts() const final {
        auto const interrupt_enables_tuple =
            irq_impls.apply([](auto... irqs_pack) {
                return stdx::tuple_cat(irqs_pack.get_interrupt_enables()...);
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
    template <irq_num_t IrqNumber> inline void run() const {
        using M =
            stdx::type_map<stdx::vt_pair<IrqImpls::irq_number, IrqImpls>...>;
        using irq_t = stdx::value_lookup_t<M, IrqNumber>;

        if constexpr (not std::is_void_v<irq_t>) {
            get<irq_t>(irq_impls).run();
        }
    }

    /**
     * @return The highest active IRQ number.
     */
    [[nodiscard]] constexpr auto max_irq() const -> irq_num_t {
        return static_cast<irq_num_t>(
            std::max({stdx::to_underlying(IrqImpls::irq_number)...}));
    }
};
} // namespace interrupt
