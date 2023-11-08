#pragma once

#include <interrupt/builder/sub_irq_builder.hpp>
#include <interrupt/impl/shared_sub_irq_impl.hpp>

#include <stdx/tuple.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace interrupt {
template <typename Config> class shared_sub_irq_builder {
    template <typename BuilderValue, std::size_t Index> struct sub_value {
        constexpr static auto const &value =
            get<Index>(BuilderValue::value.irqs);
    };

    template <typename BuilderValue, auto... Is>
    constexpr static auto built_irqs(std::index_sequence<Is...>) {
        return stdx::make_tuple(
            get<Is>(BuilderValue::value.irqs)
                .template build<sub_value<BuilderValue, Is>>()...);
    }

  public:
    constexpr static auto resources = Config::resources;
    using irq_callback_t = typename Config::irq_callback_t;
    constexpr static auto children = Config::children;

    constexpr static auto irqs_type = stdx::transform(
        [](auto child) {
            if constexpr (decltype(child.children)::size() > 0u) {
                return shared_sub_irq_builder<decltype(child)>{};
            } else {
                return sub_irq_builder<decltype(child)>{};
            }
        },
        Config::children);

    std::remove_cv_t<decltype(irqs_type)> irqs;

    template <typename IrqType, typename T>
    constexpr void add(T const &flow_description) {
        stdx::for_each(
            [&](auto &irq) { irq.template add<IrqType>(flow_description); },
            irqs);
    }

    /**
     * @return shared_irq::impl specialization optimized for size and runtime.
     */
    template <typename BuilderValue>
    [[nodiscard]] constexpr auto build() const {
        using sub_irqs_t = decltype(BuilderValue::value.irqs);
        auto const sub_irq_impls = built_irqs<BuilderValue>(
            std::make_index_sequence<sub_irqs_t::size()>{});

        return sub_irq_impls.apply([](auto... sub_irq_impl_args) {
            return shared_sub_irq_impl<Config, decltype(sub_irq_impl_args)...>(
                sub_irq_impl_args...);
        });
    }
};
} // namespace interrupt
