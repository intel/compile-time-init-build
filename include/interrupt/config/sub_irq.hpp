#pragma once

#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>

namespace interrupt {
/**
 * Declare a sub-interrupt under a shared interrupt.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of shared_irq.
 *
 * @tparam EnableField
 *      The croo register field type to enable/disable this interrupt.
 *
 * @tparam StatusField
 *      The croo register field that indicates if this interrupt is pending or
 * not.
 */
template <typename EnableField, typename StatusField, typename IrqCallback,
          typename Policies>
struct sub_irq {
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
    using irq_callback_t = IrqCallback;
    constexpr static auto children = stdx::tuple{};
};
} // namespace interrupt
