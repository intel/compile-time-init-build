#pragma once

#include <cib/tuple.hpp>
#include <interrupt/builder/shared_sub_irq_builder.hpp>
#include <interrupt/builder/sub_irq_builder.hpp>
#include <interrupt/impl/shared_irq_impl.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace interrupt {
/**
 * Declare a shared interrupt with one or more SubIrqs.
 *
 * A shared interrupt declares one hardware irq that may be caused by one or
 * more different sub-interrupts. When a shared_irq is triggered, it will
 * determine which sub_irq needs to be triggered.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of shared_irq.
 */
template <typename ConfigT> class shared_irq_builder {
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
    constexpr static auto children = ConfigT::children;

    constexpr static auto irqs_type = cib::transform(
        [](auto child) {
            if constexpr (decltype(child.children)::size() > 0u) {
                return shared_sub_irq_builder<decltype(child)>{};
            } else {
                return sub_irq_builder<decltype(child)>{};
            }
        },
        ConfigT::children);

    std::remove_cv_t<decltype(irqs_type)> irqs;

    template <typename IrqType, typename T>
    void constexpr add(T const &flow_description) {
        cib::for_each(
            [&](auto &irq) { irq.template add<IrqType>(flow_description); },
            irqs);
    }

    /**
     * @return shared_irq::impl specialization optimized for size and runtime.
     */
    template <typename BuilderValue>
    [[nodiscard]] auto constexpr build() const {
        using irqs_t = decltype(BuilderValue::value.irqs);
        auto const sub_irq_impls = built_irqs<BuilderValue>(
            std::make_index_sequence<irqs_t::size()>{});

        return sub_irq_impls.apply([](auto... sub_irq_impl_args) {
            return shared_irq_impl<ConfigT, decltype(sub_irq_impl_args)...>(
                sub_irq_impl_args...);
        });
    }
};
} // namespace interrupt
