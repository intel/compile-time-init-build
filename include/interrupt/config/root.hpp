#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace interrupt {
template <typename... Irqs> struct root {
    using status_policy_t = clear_status_first;
    constexpr static auto resources = stdx::tuple{};
    constexpr static auto children = stdx::tuple<Irqs...>{};

  private:
    template <typename Irq> constexpr static auto getAllIrqs(Irq irq) {
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
