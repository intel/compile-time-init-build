#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
template <typename EnableField, typename StatusField, typename PoliciesT,
          typename... SubIrqs>
struct shared_sub_irq {
    template <bool en> constexpr static FunctionPtr enable_action = [] {};
    constexpr static auto enable_field = EnableField{};
    constexpr static auto status_field = StatusField{};
    using StatusPolicy = typename PoliciesT::template type<status_clear_policy,
                                                           clear_status_first>;
    constexpr static auto resources =
        PoliciesT::template get<required_resources_policy,
                                required_resources<>>()
            .resources;
    using IrqCallbackType = void;
    constexpr static stdx::tuple<SubIrqs...> children{};
};
} // namespace interrupt
