#pragma once

#include <flow/flow.hpp>
#include <interrupt/builder/irq_builder.hpp>
#include <interrupt/builder/shared_irq_builder.hpp>
#include <interrupt/builder/shared_sub_irq_builder.hpp>
#include <interrupt/builder/sub_irq_builder.hpp>
#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/impl/manager_impl.hpp>
#include <interrupt/manager_interface.hpp>
#include <interrupt/policies.hpp>

#include <boost/hana.hpp>

#include <tuple>
#include <type_traits>

namespace interrupt {

/**
 * Declare one or more Irqs, SharedIrqs, and their corresponding interrupt
 * service routine attachment points.
 *
 * @tparam InterruptHal
 *      The hardware abstraction layer that knows how to clear pending interrupt
 * status.
 *
 * @tparam IRQs
 */
template <typename RootT, typename ConcurrencyPolicyT> class manager {
  public:
    using InterruptHal = typename RootT::InterruptHal;

    constexpr static auto irqs_type =
        hana::transform(RootT::children, [](auto child) {
            if constexpr (hana::size(child.children) > hana::size_c<0>) {
                return shared_irq_builder<decltype(child)>{};
            } else {
                return irq_builder<decltype(child)>{};
            }
        });

    std::remove_cv_t<decltype(irqs_type)> irqs;

    using Dynamic = dynamic_controller<RootT, ConcurrencyPolicyT>;

    /**
     * Add interrupt service routine(s) to be executed when this IRQ is
     * triggered.
     *
     * @tparam IrqType
     *      The IrqType the flow_description should be attached to.
     *
     * @param flow_description
     *      See flow::Builder<>.add()
     */
    template <typename IrqType, typename T>
    void constexpr add(T const &flow_description) {
        hana::for_each(irqs, [&](auto &irq) {
            irq.template add<IrqType>(flow_description);
        });
    }

    ///////////////////////////////////////////////////////////////////////////
    ///
    /// Everything below is for the cib extension interface. It lets cib know
    /// this builder supports the cib pattern and how to build it.
    ///
    ///////////////////////////////////////////////////////////////////////////
    /**
     * Never called, but the return type is used by cib to determine what the
     * abstract interface is.
     */
    [[nodiscard]] manager_interface *base() const;

    template <typename BuilderValue, typename Index> struct sub_value {
        constexpr static auto const &value = BuilderValue::value.irqs[Index{}];
    };

    /**
     * Given a constexpr Manager instance stored in BuilderValue::value, build
     * an optimal Manager::impl instance to initialize and run interrupts at
     * runtime.
     *
     * @tparam BuilderValue
     *      A type with a static constexpr Manager field to be built into a
     * Manager::impl
     *
     * @return The optimized Manager::impl to be used at runtime.
     */
    template <typename BuilderValue>
    [[nodiscard]] static auto constexpr build() {
        auto constexpr builder = BuilderValue::value;

        auto constexpr irq_indices = hana::to<hana::tuple_tag>(
            hana::make_range(hana::int_c<0>, hana::size(builder.irqs)));

        auto const irq_impls = hana::transform(irq_indices, [&](auto i) {
            constexpr auto irq = builder.irqs[i];
            return irq.template build<sub_value<BuilderValue, decltype(i)>>();
        });

        return hana::unpack(irq_impls, [](auto... irq_impl_args) {
            return manager_impl<InterruptHal, Dynamic,
                                decltype(irq_impl_args)...>(irq_impl_args...);
        });
    }
};
} // namespace interrupt
