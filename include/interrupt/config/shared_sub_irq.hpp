#include <interrupt/config/fwd.hpp>
#include <interrupt/policies.hpp>

#include <boost/hana.hpp>

#ifndef CIB_INTERRUPT_CONFIG_SHARED_SUB_IRQ_HPP
#define CIB_INTERRUPT_CONFIG_SHARED_SUB_IRQ_HPP

namespace interrupt {
namespace hana = boost::hana;
using namespace hana::literals;

template <typename EnableField, typename StatusField, typename PoliciesT,
          typename... SubIrqs>
struct shared_sub_irq {
    template <typename InterruptHal, bool en>
    constexpr static EnableActionType enable_action = []() {};
    constexpr static auto enable_field = hana::just(EnableField{});
    constexpr static auto status_field = hana::just(StatusField{});
    using StatusPolicy = typename PoliciesT::template type<status_clear_policy,
                                                           clear_status_first>;
    constexpr static auto resources =
        PoliciesT::template get<required_resources_policy,
                                required_resources<>>()
            .resources;
    using IrqCallbackType = void;
    constexpr static hana::tuple<SubIrqs...> children{};
};
} // namespace interrupt

#endif // CIB_INTERRUPT_CONFIG_SHARED_SUB_IRQ_HPP
