#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <interrupt/config/fwd.hpp>
#include <interrupt/policies.hpp>

namespace interrupt {
template <typename InterruptHalT, typename... IrqsT> struct root {
    using InterruptHal = InterruptHalT;

    template <typename T, bool en>
    constexpr static EnableActionType enable_action = []() {};
    using StatusPolicy = clear_status_first;
    constexpr static auto resources = cib::make_tuple();
    using IrqCallbackType = void;
    constexpr static cib::tuple<IrqsT...> children{};

  private:
    template <typename IrqType> constexpr static auto getAllIrqs(IrqType irq) {
        return irq.children.apply([&](auto... irqChildren) {
            return cib::tuple_cat(getAllIrqs(irqChildren)...,
                                  cib::make_tuple(irq));
        });
    }

  public:
    constexpr static auto all_irqs = children.apply([](auto... irqChildren) {
        return cib::tuple_cat(getAllIrqs(irqChildren)...);
    });
};
} // namespace interrupt
