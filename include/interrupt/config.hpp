#pragma once

// FIXME: includes

#include <interrupt/policies.hpp>

namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;


    using EnableActionType = void(*)();

    /**
     * Declare a simple unshared interrupt.
     *
     * This object is designed only to live in a constexpr context. The template specialization
     * should be declared by the user while the interrupt::Manager creates and manages
     * instances of irq.
     *
     * @tparam IrqNumberT
     *      Hardware IRQ number.
     *
     * @tparam IrqPriorityT
     *      Hardware IRQ priority.
     */
    template<
        std::size_t IrqNumberT,
        std::size_t IrqPriorityT,
        typename IrqCallbackT,
        typename PoliciesT>
    struct irq {
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = InterruptHal::template irqInit<en, IrqNumberT, IrqPriorityT>;
        constexpr static auto enable_field = hana::nothing;
        constexpr static auto status_field = hana::nothing;
        using StatusPolicy = typename PoliciesT::template type<status_clear_policy, clear_status_first>;
        constexpr static auto resources = PoliciesT::template get<required_resources_policy, required_resources<>>().resources;
        using IrqCallbackType = IrqCallbackT;
        constexpr static hana::tuple<> children{};

        constexpr static auto irq_number = IrqNumberT;
    };


    /**
     * Declare a shared interrupt with one or more SubIrqs.
     *
     * A shared interrupt declares one hardware irq that may be caused by one or more different
     * sub-interrupts. When a shared_irq is triggered, it will determine which sub_irq needs to be
     * triggered.
     *
     * This object is designed only to live in a constexpr context. The template specialization
     * should be declared by the user while the interrupt::Manager creates and manages
     * instances of shared_irq.
     *
     * @tparam IrqNumberT
     *      Hardware IRQ number.
     *
     * @tparam IrqPriorityT
     *      Hardware IRQ priority.
     *
     * @tparam SubIrqs
     *      One or more sub_irq types in this shared_irq.
     */
    template<
        std::size_t IrqNumberT,
        std::size_t IrqPriorityT,
        typename PoliciesT,
        typename... SubIrqs>
    struct shared_irq {
    public:
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = InterruptHal::template irqInit<en, IrqNumberT, IrqPriorityT>;
        constexpr static auto enable_field = hana::nothing;
        constexpr static auto status_field = hana::nothing;
        using StatusPolicy = typename PoliciesT::template type<status_clear_policy, clear_status_first>;
        constexpr static auto resources = PoliciesT::template get<required_resources_policy, required_resources<>>().resources;
        using IrqCallbackType = void;
        constexpr static hana::tuple<SubIrqs...> children{};

        constexpr static auto irq_number = IrqNumberT;
    };

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


    template<
        typename EnableField,
        typename StatusField,
        typename PoliciesT,
        typename... SubIrqs>
    struct shared_sub_irq {
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = [](){};
        constexpr static auto enable_field = hana::just(EnableField{});
        constexpr static auto status_field = hana::just(StatusField{});
        using StatusPolicy = typename PoliciesT::template type<status_clear_policy, clear_status_first>;
        constexpr static auto resources = PoliciesT::template get<required_resources_policy, required_resources<>>().resources;
        using IrqCallbackType = void;
        constexpr static hana::tuple<SubIrqs...> children{};
    };

    template<
        typename InterruptHalT,
        typename... IrqsT>
    struct root {
        using InterruptHal = InterruptHalT;

        template<typename T, bool en>
        constexpr static EnableActionType enable_action = [](){};
        constexpr static auto enable_field = hana::nothing;
        constexpr static auto status_field = hana::nothing;
        using StatusPolicy = clear_status_first;
        constexpr static auto resources = hana::make_tuple();
        using IrqCallbackType = void;
        constexpr static hana::tuple<IrqsT...> children{};

    private:
        template<typename IrqType>
        constexpr static auto getAllIrqs(IrqType irq) {
            auto const descendants =
                hana::unpack(irq.children, [](auto... irqChildren){
                    return hana::flatten(hana::make_tuple(getAllIrqs(irqChildren)...));
                });

            return hana::append(descendants, irq);
        }

    public:
        constexpr static auto all_irqs =
            hana::unpack(children, [](auto... irqChildren){
                return hana::flatten(hana::make_tuple(getAllIrqs(irqChildren)...));
            });
    };
}
