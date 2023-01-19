#pragma once

#include <cib/tuple.hpp>
#include <flow/builder.hpp>
#include <interrupt/builder/irq_builder.hpp>
#include <interrupt/builder/shared_irq_builder.hpp>
#include <interrupt/builder/shared_sub_irq_builder.hpp>
#include <interrupt/builder/sub_irq_builder.hpp>
#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/impl/manager_impl.hpp>
#include <interrupt/manager_interface.hpp>
#include <interrupt/policies.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace interrupt {
template <typename Name = void> using irq_flow = flow::builder<Name, 16, 8>;

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
    template <typename BuilderValue, std::size_t Index> struct sub_value {
        constexpr static auto const &value =
            cib::get<Index>(BuilderValue::value.irqs);
    };

    template <typename BuilderValue, auto... Is>
    constexpr static auto built_irqs(std::index_sequence<Is...>) {
        return cib::make_tuple(
            cib::get<Is>(BuilderValue::value.irqs)
                .template build<sub_value<BuilderValue, Is>>()...);
    }

  public:
    using InterruptHal = typename RootT::InterruptHal;

    constexpr static auto irqs_type = cib::transform(
        [](auto child) {
            if constexpr (decltype(child.children)::size() > 0u) {
                return shared_irq_builder<decltype(child)>{};
            } else {
                return irq_builder<decltype(child)>{};
            }
        },
        RootT::children);

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
        cib::for_each(
            [&](auto &irq) { irq.template add<IrqType>(flow_description); },
            irqs);
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
    [[nodiscard]] auto base() const -> manager_interface *;

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
        using irqs_t = decltype(BuilderValue::value.irqs);
        auto const irq_impls = built_irqs<BuilderValue>(
            std::make_index_sequence<irqs_t::size()>{});

        return cib::apply(
            [](auto... irq_impl_args) {
                return manager_impl<InterruptHal, Dynamic,
                                    decltype(irq_impl_args)...>(
                    irq_impl_args...);
            },
            irq_impls);
    }
};
} // namespace interrupt
