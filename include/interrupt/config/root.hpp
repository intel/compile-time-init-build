#include <interrupt/policies.hpp>
#include <interrupt/config/fwd.hpp>
#include <boost/hana.hpp>


#ifndef CIB_INTERRUPT_CONFIG_ROOT_HPP
#define CIB_INTERRUPT_CONFIG_ROOT_HPP


namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;

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


#endif //CIB_INTERRUPT_CONFIG_ROOT_HPP
