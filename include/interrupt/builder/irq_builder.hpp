#include <interrupt/config/fwd.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/impl/irq_impl.hpp>

#include <boost/hana.hpp>

#include <type_traits>

#ifndef CIB_INTERRUPT_BUILDER_IRQ_BUILDER_HPP
#define CIB_INTERRUPT_BUILDER_IRQ_BUILDER_HPP

namespace interrupt {
/**
 * Declare a simple unshared interrupt.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of irq.
 *
 * @tparam IrqNumberT
 *      Hardware IRQ number.
 *
 * @tparam IrqPriorityT
 *      Hardware IRQ priority.
 */
template <typename ConfigT> struct irq_builder {
    using IrqCallbackType = typename ConfigT::IrqCallbackType;
    constexpr static auto irq_number = ConfigT::irq_number;

  private:
    IrqCallbackType interrupt_service_routine;

  public:
    /**
     * Add interrupt service routine(s) to be executed when this IRQ is
     * triggered.
     *
     * This should be used only by interrupt::Manager.
     *
     * @param flow_description
     *      See flow::Builder<>.add()
     */
    template <typename IrqType, typename T>
    void constexpr add(T const &flow_description) {
        if constexpr (std::is_same_v<IrqCallbackType, IrqType>) {
            interrupt_service_routine.add(flow_description);
        }
    }

    /**
     * @return irq::impl specialization optimized for size and runtime.
     */
    template <typename BuilderValue>
    [[nodiscard]] auto constexpr build() const {
        auto constexpr run_flow = [] {
            auto constexpr flow_builder =
                BuilderValue::value.interrupt_service_routine;
            auto constexpr flow_size = flow_builder.size();
            auto constexpr flow =
                flow_builder.template internal_build<flow_size>();
            flow();
        };

        auto constexpr flow_builder =
            BuilderValue::value.interrupt_service_routine;
        auto constexpr flow_size = flow_builder.size();
        auto const optimized_irq_impl =
            irq_impl<ConfigT,
                     flow::impl<typename IrqCallbackType::Name, flow_size>>(
                run_flow);

        return optimized_irq_impl;
    }
};
} // namespace interrupt

#endif // CIB_INTERRUPT_BUILDER_IRQ_BUILDER_HPP
