#pragma once

#include <cib/tuple.hpp>
#include <interrupt/config/fwd.hpp>
#include <interrupt/policies.hpp>

namespace interrupt {
template <typename EnableField, typename StatusField, typename PoliciesT,
          typename... SubIrqs>
struct shared_sub_irq {
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action = []() {};
    constexpr static auto enable_field = EnableField{};
    constexpr static auto status_field = StatusField{};
    using StatusPolicy = typename PoliciesT::template type<status_clear_policy,
                                                           clear_status_first>;
    constexpr static auto resources =
        PoliciesT::template get<required_resources_policy,
                                required_resources<>>()
            .resources;
    using IrqCallbackType = void;
    constexpr static cib::tuple<SubIrqs...> children{};
};
} // namespace interrupt
