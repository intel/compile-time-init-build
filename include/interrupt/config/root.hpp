#pragma once

#include <interrupt/config/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace interrupt {
template <typename InterruptHalT, typename... IrqsT> struct root {
    using InterruptHal = InterruptHalT;

    template <typename T, bool en>
    constexpr static EnableActionType enable_action = []() {};
    using StatusPolicy = clear_status_first;
    constexpr static auto resources = stdx::make_tuple();
    using IrqCallbackType = void;
    constexpr static stdx::tuple<IrqsT...> children{};

  private:
    template <typename IrqType> constexpr static auto getAllIrqs(IrqType irq) {
        return irq.children.apply([&](auto... irqChildren) {
            return stdx::tuple_cat(getAllIrqs(irqChildren)...,
                                   stdx::make_tuple(irq));
        });
    }

  public:
    constexpr static auto all_irqs = children.apply([](auto... irqChildren) {
        return stdx::tuple_cat(getAllIrqs(irqChildren)...);
    });
};
} // namespace interrupt
