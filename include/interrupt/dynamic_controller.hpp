#pragma once

#include <cib/detail/compiler.hpp>

#include <boost/hana.hpp>

#include <type_traits>

#include <Concurrency.hpp>


namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;

    enum class resource_status {
        OFF = 0,
        ON  = 1
    };

    template<typename RootT>
    struct dynamic_controller {
    private:
        /**
         * Store the interrupt enable values that are allowed given the current set of resources that are available.
         *
         * @tparam RegType
         *      The croo::Register this mask corresponds to.
         */
        template<typename RegType>
        CIB_CONSTINIT static inline typename RegType::DataType allowed_enables =
            std::numeric_limits<typename RegType::DataType>::max();

        /**
         * For each ResourceType, keep track of what interrupts can still be enabled when that resource goes down.
         *
         * Each bit in this mask corresponds to an interrupt enable field in RegType. If the bit is '1', that means
         * the corresponding interrupt can be enabled when the resource is not available. If the bit is '0', that
         * means the corresonding interrupt must be disabled when the resource is not available.
         *
         * @tparam ResourceType
         *      The resource we want to check.
         *
         * @tparam RegType
         *      The specific register mask we want to check.
         */
        template<typename ResourceType, typename RegType>
        constexpr static typename RegType::DataType irqs_allowed = [](){
            // get all interrupt enable fields that don't require the given resource
            auto const matching_irqs =
                hana::filter(RootT::all_irqs, [](auto irq){
                    using IrqType = decltype(irq);
                    constexpr auto doesnt_require_resource =
                        hana::unpack(irq.resources, [](auto... resources_pack){
                            return (!std::is_same_v<decltype(resources_pack), ResourceType> && ...);
                        });

                    constexpr auto has_enable_field = irq.enable_field != hana::nothing;
                    return hana::bool_c<doesnt_require_resource && has_enable_field>;
                });

            auto const interrupt_enables_tuple =
                hana::transform(matching_irqs, [](auto irq){
                    return *irq.enable_field;
                });

            // filter fields that aren't in RegType
            auto const fields_in_reg =
                hana::filter(interrupt_enables_tuple, [](auto field){
                    using FieldsRegType = typename decltype(field)::RegisterType;
                    return hana::bool_c<std::is_same_v<FieldsRegType, RegType>>;
                });

            // set the bits in the mask for interrupts that don't require the resource
            using DataType = typename RegType::DataType;
            return hana::fold(fields_in_reg, DataType{}, [](DataType value, auto field){
                return value | field.get_mask();
            });
        }();

        template<typename ResourceType>
        CIB_CONSTINIT static inline bool is_resource_on = true;

        template<typename RegTypeTuple>
        static inline void reprogram_interrupt_enables(RegTypeTuple regs) {
            hana::for_each(regs, [](auto reg){
                using RegType = decltype(reg);

                // make sure we don't enable any interrupts that are not allowed according to resource availability
                auto const final_enables = allowed_enables<RegType> & dynamic_enables<RegType>;

                // update the hardware registers
                apply(write(reg.raw(final_enables)));
            });
        }

        template<typename RegFieldTuple>
        constexpr static auto get_unique_regs(RegFieldTuple fields) {
            return hana::fold(fields, hana::make_tuple(), [](auto regs, auto field){
                constexpr bool reg_has_been_seen_already =
                    hana::unpack(decltype(regs){}, [=](auto... regs_pack){
                        return (std::is_same_v<typename decltype(field)::RegisterType, decltype(regs_pack)> || ...);
                    });

                if constexpr (reg_has_been_seen_already) {
                    return regs;
                } else {
                    return hana::append(regs, field.get_register());
                }
            });
        }

        /**
         * hana::tuple of every resource mentioned in the interrupt configuration
         */
        constexpr static auto all_resources =
            hana::fold(RootT::all_irqs, hana::make_tuple(), [](auto resources, auto irq){
                // TODO: check that an IRQ doesn't list a resource more than once
                auto const additional_resources =
                    hana::filter(irq.resources, [=](auto resource){
                        constexpr bool match =
                            hana::unpack(resources, [](auto... resources_pack){
                                return (std::is_same_v<decltype(resource), decltype(resources_pack)> || ...);
                            });

                        return hana::bool_c<!match>;
                    });

                return hana::concat(resources, additional_resources);
            });

        /**
         * hana::tuple of every interrupt register affected by a resource 
         */
        constexpr static auto all_resource_affected_regs =
            get_unique_regs(hana::fold(RootT::all_irqs, hana::make_tuple(), [](auto registers, auto irq){
                constexpr bool has_enable_field = irq.enable_field != hana::nothing;
                constexpr bool depends_on_resources = hana::size(irq.resources) > hana::size_c<0>;
                if constexpr (has_enable_field && depends_on_resources) {
                    return hana::append(registers, irq.enable_field->get_register());
                } else {
                    return registers;
                }
            }));

        /**
         * Reprogram interrupt enables based on updated resource availability.
         */
        static inline auto recalculate_allowed_enables() {
            // set allowed_enables mask for each resource affected register
            hana::for_each(all_resource_affected_regs, [](auto reg){
                using RegType = decltype(reg);
                using DataType = typename RegType::DataType;
                allowed_enables<RegType> = std::numeric_limits<DataType>::max();
            });

            // for each resource, if it is not on, mask out unavailable interrupts
            hana::for_each(all_resources, [=](auto resource){
                using ResourceType = decltype(resource);
                if (is_resource_on<ResourceType> == false) {
                    hana::for_each(all_resource_affected_regs, [](auto reg){
                        using RegType = decltype(reg);
                        allowed_enables<RegType> &= irqs_allowed<ResourceType, RegType>;
                    });
                }
            });

            return all_resource_affected_regs;
        }

        /**
         * Store the interrupt enable values that FW _wants_ at runtime, irrespective of any resource conflicts
         * that would require specific interrupts to be disabled.
         *
         * @tparam RegType
         *      The croo::Register this value corresponds to.
         */
        template<typename RegType>
        CIB_CONSTINIT static inline typename RegType::DataType dynamic_enables{};

        template<bool en, typename... CallbacksToFind>
        static inline void enable_by_name() {
            // NOTE: critical section is not needed here because shared state is only
            //       updated by the final call to enable_by_field

            // TODO: add support to enable/disable top-level IRQs by name.
            //       this will require another way to manage them vs. mmio registers.
            //       once that goes in, then enable_by_field should be removed or made private.
            auto const matching_irqs =
                hana::filter(RootT::all_irqs, [](auto irq) {
                    using IrqType = decltype(irq);
                    using IrqCallbackType = typename IrqType::IrqCallbackType;
                    constexpr auto has_callback = (std::is_same_v<CallbacksToFind, IrqCallbackType> || ...);
                    constexpr auto has_enable_field = irq.enable_field != hana::nothing;
                    return hana::bool_c<has_callback && has_enable_field>;
                });

            auto const interrupt_enables_tuple =
                hana::transform(matching_irqs, [](auto irq) {
                    return *irq.enable_field;
                });

            hana::unpack(interrupt_enables_tuple, [](auto... fields) {
                enable_by_field<en, decltype(fields)...>();
            });
        }

    public:
        template<typename ResourceType>
        static inline void update_resource(resource_status status) {
            conc::critical_section([&]{
                is_resource_on<ResourceType> = (status == resource_status::ON);
                recalculate_allowed_enables();
                reprogram_interrupt_enables(all_resource_affected_regs);
            });
        }

        template<typename ResourceType>
        static inline void turn_on_resource() {
            update_resource<ResourceType>(resource_status::ON);
        }

        template<typename ResourceType>
        static inline void turn_off_resource() {
            update_resource<ResourceType>(resource_status::OFF);
        }

        template<bool en, typename... FieldsToSet>
        static inline void enable_by_field() {
            auto const interrupt_enables_tuple =
                hana::tuple<FieldsToSet...>{};

            conc::critical_section([&] {
                // update the dynamic enables
                if constexpr (en) {
                    hana::for_each(interrupt_enables_tuple, [](auto f) {
                        using RegType = decltype(f.get_register());
                        dynamic_enables<RegType> |= f.get_mask();
                    });
                } else {
                    hana::for_each(interrupt_enables_tuple, [](auto f) {
                        using RegType = decltype(f.get_register());
                        dynamic_enables<RegType> &= ~f.get_mask();
                    });
                }

                auto const unique_regs = get_unique_regs(interrupt_enables_tuple);
                reprogram_interrupt_enables(unique_regs);
            });
        }

        template<typename... CallbacksToFind>
        static inline void  enable() {
            enable_by_name<true, CallbacksToFind...>();
        }

        template<typename... CallbacksToFind>
        static inline void disable() {
            enable_by_name<false, CallbacksToFind...>();
        }
    };
}