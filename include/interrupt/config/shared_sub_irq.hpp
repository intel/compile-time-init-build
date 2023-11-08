#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
template <typename EnableField, typename StatusField, typename Policies,
          typename... SubIrqs>
struct shared_sub_irq {
    template <bool> constexpr static FunctionPtr enable_action = [] {};
    constexpr static auto enable_field = EnableField{};
    constexpr static auto status_field = StatusField{};
    using status_policy_t =
        typename Policies::template type<status_clear_policy,
                                         clear_status_first>;
    constexpr static auto resources =
        Policies::template get<required_resources_policy,
                               required_resources<>>()
            .resources;
    using irq_callback_t = void;
    constexpr static auto children = stdx::tuple<SubIrqs...>{};
};
} // namespace interrupt
