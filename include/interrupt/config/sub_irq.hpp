#include <interrupt/policies.hpp>
#include <interrupt/config/fwd.hpp>
#include <boost/hana.hpp>


#ifndef CIB_INTERRUPT_CONFIG_SUB_IRQ_HPP
#define CIB_INTERRUPT_CONFIG_SUB_IRQ_HPP


namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;

    /**
     * Declare a sub-interrupt under a shared interrupt.
     *
     * This object is designed only to live in a constexpr context. The template specialization
     * should be declared by the user while the interrupt::Manager creates and manages
     * instances of shared_irq.
     *
     * @tparam EnableField
     *      The croo register field type to enable/disable this interrupt.
     *
     * @tparam StatusField
     *      The croo register field that indicates if this interrupt is pending or not.
     */
    template<
        typename EnableField,
        typename StatusField,
        typename IrqCallbackT,
        typename PoliciesT>
    struct sub_irq {
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = [](){};
        constexpr static auto enable_field = hana::just(EnableField{});
        constexpr static auto status_field = hana::just(StatusField{});
        using StatusPolicy = typename PoliciesT::template type<status_clear_policy, clear_status_first>;
        constexpr static auto resources = PoliciesT::template get<required_resources_policy, required_resources<>>().resources;
        using IrqCallbackType = IrqCallbackT;
        constexpr static hana::tuple<> children{};
    };
}


#endif //CIB_INTERRUPT_CONFIG_SUB_IRQ_HPP
