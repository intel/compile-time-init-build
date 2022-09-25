#pragma once

#include <flow/flow.hpp>

#include <boost/hana.hpp>

#include <type_traits>
#include <tuple>

// FIXME: includes
#include <interrupt/policies.hpp>
#include <interrupt/config.hpp>

#include <interrupt/dynamic_controller.hpp>

namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;
    using FunctionPtr = std::add_pointer<void()>::type;

    /*
     * Preface
     * -------
     *
     * interrupt::Manager makes heavy use of the boost::hana library.
     *
     * Take a look at the introduction online:
     * https://www.boost.org/doc/libs/1_65_1/libs/hana/doc/html/index.html#tutorial-introduction
     *
     * There's also a cheatsheet for quick reference:
     * https://www.boost.org/doc/libs/1_65_1/libs/hana/doc/html/index.html#tutorial-cheatsheet
     *
     * Of the entire library, hana::tuple, hana::range, and hana::integral_constant types along
     * with hana::transform, hana::zip, hana::unpack, hana::filter, and hana::for_each functions
     * are used often. These can be referenced from the hana cheatsheet for a basic understanding.
     */

    /*
     * Introduction
     * ------------
     *
     * interrupt::Manager is intended to automate and hide the low level details of interrupt
     * hardware initialization, registration, and execution while using the least amount of memory
     * and execution time.
     *
     * The interrupt configuration is declared using a template specialization of
     * interrupt::Manager. The template parameters define the interrupt numbers, priorities,
     * and shared irq parameters like enable and status registers.
     *
     * Each irq contains a flow::impl for registering interrupt service routines.
     *
     * After the interrupt::Manager is declared and interrupt service routines added in a
     * constexpr initialization routine, the interrupt::Manager.build() function is called
     * to generate the interrupt::ManagerImpl to be used at runtime.
     *
     * The ManagerImpl.init() method enables and initializes all interrupts with associated
     * interrupt service routines registered.
     *
     * The ManagerImpl.run<irq_number>() method clears the pending interrupt status and runs any
     * registered interrupt service routines for that IRQ. If it's a shared interrupt, each
     * registered sub_irq will have its interrupt status field checked to determine if its
     * interrupt service routines should be executed.
     */

    template<typename Name = void>
    using irq_flow = flow::builder<Name, 16, 8>;




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
    template<typename ConfigT>
    struct irq_builder {
    public:
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = ConfigT::template enable_action<InterruptHal, en>;
        constexpr static auto enable_field = ConfigT::enable_field;
        constexpr static auto status_field = ConfigT::status_field;
        using StatusPolicy = typename ConfigT::StatusPolicy;
        constexpr static auto resources = ConfigT::resources;
        using IrqCallbackType = typename ConfigT::IrqCallbackType;
        constexpr static auto children = ConfigT::children;

        constexpr static auto irq_number = ConfigT::irq_number;

    private:
        IrqCallbackType interrupt_service_routine;

    public:
        /**
         * Add interrupt service routine(s) to be executed when this IRQ is triggered.
         *
         * This should be used only by interrupt::Manager.
         *
         * @param flow_description
         *      See flow::Builder<>.add()
         */
        template<typename IrqType, typename T>
        void constexpr add(T const & flow_description) {
            if constexpr (std::is_same_v<IrqCallbackType, IrqType>) {
                interrupt_service_routine.add(flow_description);
            }
        }

        /**
         * @return irq::impl specialization optimized for size and runtime.
         */
        template<typename BuilderValue>
        [[nodiscard]] auto constexpr build() const {
            auto constexpr flow_builder = BuilderValue::value.interrupt_service_routine;
            auto constexpr flow_size = flow_builder.size();

            auto constexpr run_flow = []{
                auto constexpr flow_builder = BuilderValue::value.interrupt_service_routine;
                auto constexpr flow_size = flow_builder.size();
                auto constexpr flow = flow_builder.template internal_build<flow_size>();
                flow();
            };

            auto const optimized_irq_impl =
                impl<flow::impl<typename IrqCallbackType::Name, flow_size>>(run_flow);

            return optimized_irq_impl;
        }

    private:
        /**
         * Runtime implementation of the irq.
         *
         * @tparam FlowTypeT
         *      The actual flow::impl<Size> type that can contain all of the interrupt service routines
         *      from the irq's flow::Builder<>. This needs to be accurately sized to ensure it can
         *      indicate whether any interrupt service routines are registered.
         */
        template<typename FlowTypeT>
        struct impl {
        public:
            constexpr static auto irq_number = ConfigT::irq_number;

            /**
             * True if this irq::impl has any interrupt service routines to execute, otherwise
             * False.
             *
             * This is used to optimize compiled size and runtime performance. Unused Irqs should
             * not consume any resources.
             */
            static bool constexpr active = FlowTypeT::active;

        private:
            FunctionPtr interrupt_service_routine;

        public:
            explicit constexpr impl(
                FunctionPtr const & flow
            )
                : interrupt_service_routine(flow)
            { }

            /**
             * Initialize and enable the hardware interrupt.
             *
             * This should be used only by interrupt::Manager.
             *
             * @tparam InterruptHal
             *      The hardware abstraction layer that knows how to initialize hardware interrupts.
             */
            template<typename InterruptHal>
            inline void init_mcu_interrupts() const {
                enable_action<InterruptHal, active>();
            }

            inline auto get_interrupt_enables() const {
                return hana::make_tuple();
            }

            /**
             * Run the interrupt service routine and clear any pending interrupt status.
             *
             * This should be used only by interrupt::Manager.
             *
             * @tparam InterruptHal
             *      The hardware abstraction layer that knows how to clear pending interrupt status.
             */
            template<typename InterruptHal>
            inline void run() const {
                if constexpr (active) {
                    InterruptHal::template run<StatusPolicy>(irq_number, [&]() {
                        interrupt_service_routine();
                    });
                }
            }
        };
    };



    /**
     * Declare a sub-interrupt under a shared interrupt.
     *
     * This object is designed only to live in a constexpr context. The template specialization
     * should be declared by the user while the interrupt::Manager creates and manages
     * instances of shared_irq.
     */
    template<typename ConfigT>
    struct sub_irq_builder {
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = ConfigT::template enable_action<InterruptHal, en>;
        constexpr static auto enable_field = ConfigT::enable_field;
        constexpr static auto status_field = ConfigT::status_field;
        using StatusPolicy = typename ConfigT::StatusPolicy;
        constexpr static auto resources = ConfigT::resources;
        using IrqCallbackType = typename ConfigT::IrqCallbackType;
        constexpr static auto children = ConfigT::children;

    private:
        IrqCallbackType interrupt_service_routine;

    public:
        /**
         * Add interrupt service routine(s) to be executed when this IRQ is triggered.
         *
         * This should be used only by interrupt::Manager.
         *
         * @param flow_description
         *      See flow::Builder<>.add()
         */
        template<typename IrqType, typename T>
        void constexpr add(T const & flow_description) {
            if constexpr (std::is_same_v<IrqCallbackType, IrqType>) {
                interrupt_service_routine.add(flow_description);
            }
        }


        /**
         * @return sub_irq::impl specialization optimized for size and runtime.
         */
        template<typename BuilderValue>
        [[nodiscard]] auto constexpr build() const {
            auto constexpr flow_builder = BuilderValue::value.interrupt_service_routine;
            auto constexpr flow_size = flow_builder.size();

            auto constexpr run_flow = []{
                auto constexpr flow_builder = BuilderValue::value.interrupt_service_routine;
                auto constexpr flow_size = flow_builder.size();
                auto constexpr flow = flow_builder.template internal_build<flow_size>();
                flow();
            };

            auto const optimized_irq_impl =
                impl<flow::impl<typename IrqCallbackType::Name, flow_size>>(run_flow);

            return optimized_irq_impl;
        }

    private:
        /**
         * Runtime implementation of the sub_irq.
         *
         * @tparam FlowTypeT
         *      The actual flow::impl<Size> type that can contain all of the interrupt service routines
         *      from the sub_irq's flow::Builder<>. This needs to be accurately sized to ensure it can
         *      indicate whether any interrupt service routines are registered.
         */
        template<typename FlowTypeT>
        struct impl {
        public:
            /**
             * True if this sub_irq::impl has any interrupt service routines to execute, otherwise
             * False.
             *
             * This is used to optimize compiled size and runtime performance. Unused SubIrqs should
             * not consume any resources.
             */
            static bool constexpr active = FlowTypeT::active;

        private:
            FunctionPtr interrupt_service_routine;

        public:
            explicit constexpr impl(
                FunctionPtr const & flow
            )
                : interrupt_service_routine(flow)
            { }

            [[nodiscard]] auto get_interrupt_enables() const {
                return hana::make_tuple(*enable_field);
            }

            /**
             * Run the interrupt service routine and clear any pending interrupt status. This
             * includes checking and clearing the interrupt status register field.
             *
             * This should be used only by interrupt::Manager.
             *
             * @tparam InterruptHal
             *      The hardware abstraction layer that knows how to clear pending interrupt status.
             */
            inline void run() const {
                if constexpr (active) {
                    if (
                        apply(read(*enable_field)) &&
                        apply(read(*status_field))
                    ) {
                        StatusPolicy::run(
                            [&]{
                                apply(clear(*status_field));
                            },
                            [&]{
                                interrupt_service_routine();
                            });
                    }
                }
            }
        };
    };




    template<typename ConfigT>
    struct shared_sub_irq_builder {
    public:
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = ConfigT::template enable_action<InterruptHal, en>;
        constexpr static auto enable_field = ConfigT::enable_field;
        constexpr static auto status_field = ConfigT::status_field;
        using StatusPolicy = typename ConfigT::StatusPolicy;
        constexpr static auto resources = ConfigT::resources;
        using IrqCallbackType = typename ConfigT::IrqCallbackType;
        constexpr static auto children = ConfigT::children;

    public:
        constexpr static auto irqs_type =
            hana::transform(ConfigT::children, [](auto child){
                if constexpr (hana::size(child.children) > hana::size_c<0>) {
                    return shared_sub_irq_builder<decltype(child)>{};
                } else {
                    return sub_irq_builder<decltype(child)>{};
                }
            });

        std::remove_cv_t<decltype(irqs_type)> irqs;


        template<typename IrqType, typename T>
        void constexpr add(T const & flow_description) {
            hana::for_each(irqs, [&](auto & irq) {
                irq.template add<IrqType>(flow_description);
            });
        }

        template<
            typename BuilderValue,
            typename Index>
        struct sub_value {
            constexpr static auto const & value = BuilderValue::value.irqs[Index{}];
        };

        /**
         * @return shared_irq::impl specialization optimized for size and runtime.
         */
        template<typename BuilderValue>
        [[nodiscard]] auto constexpr build() const {
            auto constexpr builder = BuilderValue::value;

            auto constexpr irq_indices =
                hana::to<hana::tuple_tag>(hana::make_range(hana::int_c<0>, hana::size(builder.irqs)));

            auto const sub_irq_impls = hana::transform(irq_indices, [&](auto i){
                constexpr auto irq = builder.irqs[i];
                return irq.template build<sub_value<BuilderValue, decltype(i)>>();
            });

            return hana::unpack(sub_irq_impls, [](auto ... sub_irq_impl_args) {
                return impl<decltype(sub_irq_impl_args)...>(sub_irq_impl_args...);
            });
        }

    private:
        template<typename... SubIrqImpls>
        struct impl {
        public:

            /**
             * True if this shared_irq::impl has any active sub_irq::Impls, otherwise False.
             *
             * This is used to optimize compiled size and runtime performance. Unused irqs should
             * not consume any resources.
             */
            static bool constexpr active = (SubIrqImpls::active || ... || false);

        private:
            hana::tuple<SubIrqImpls...> sub_irq_impls;

        public:
            explicit constexpr impl(
                SubIrqImpls const & ... sub_irq_impls
            )
                :  sub_irq_impls(sub_irq_impls...)
            { }

            auto get_interrupt_enables() const {
                if constexpr (active) {
                    auto const active_sub_irq_impls =
                        hana::filter(sub_irq_impls, [](auto irq) {
                            return hana::bool_c<decltype(irq)::active>;
                        });

                    auto const sub_irq_interrupt_enables =
                        hana::unpack(active_sub_irq_impls, [](auto && ... irqs) {
                            return hana::flatten(hana::make_tuple(irqs.get_interrupt_enables()...));
                        });

                    return hana::append(sub_irq_interrupt_enables, *enable_field);

                } else {
                    return hana::make_tuple();
                }
            }

            /**
             * Evaluate interrupt status of each sub_irq::impl and run each one with a pending
             * interrupt. Clear any hardware interrupt pending bits as necessary.
             */
            inline void run() const {
                if constexpr (active) {
                    if (
                        apply(read((*enable_field)(1))) &&
                        apply(read((*status_field)(1)))
                    ) {
                        StatusPolicy::run(
                            [&]{
                                apply(clear(*status_field));
                            },
                            [&]{
                                hana::for_each(sub_irq_impls, [](auto irq) {
                                    irq.run();
                                });
                            });
                    }
                }
            }
        };
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
     */
    template<typename ConfigT>
    struct shared_irq_builder {
        template<typename InterruptHal, bool en>
        constexpr static EnableActionType enable_action = ConfigT::template enable_action<InterruptHal, en>;
        constexpr static auto enable_field = ConfigT::enable_field;
        constexpr static auto status_field = ConfigT::status_field;
        using StatusPolicy = typename ConfigT::StatusPolicy;
        constexpr static auto resources = ConfigT::resources;
        using IrqCallbackType = typename ConfigT::IrqCallbackType;
        constexpr static auto children = ConfigT::children;

        constexpr static auto irq_number = ConfigT::irq_number;


        constexpr static auto irqs_type =
            hana::transform(ConfigT::children, [](auto child){
                if constexpr (hana::size(child.children) > hana::size_c<0>) {
                    return shared_sub_irq_builder<decltype(child)>{};
                } else {
                    return sub_irq_builder<decltype(child)>{};
                }
            });

        std::remove_cv_t<decltype(irqs_type)> irqs;

        template<typename IrqType, typename T>
        void constexpr add(T const & flow_description) {
            hana::for_each(irqs, [&](auto & irq) {
                irq.template add<IrqType>(flow_description);
            });
        }

        template<
            typename BuilderValue,
            typename Index>
        struct sub_value {
            constexpr static auto const & value = BuilderValue::value.irqs[Index{}];
        };

        /**
         * @return shared_irq::impl specialization optimized for size and runtime.
         */
        template<typename BuilderValue>
        [[nodiscard]] auto constexpr build() const {
            auto constexpr builder = BuilderValue::value;

            auto constexpr irq_indices =
                hana::to<hana::tuple_tag>(hana::make_range(hana::int_c<0>, hana::size(builder.irqs)));

            auto const sub_irq_impls = hana::transform(irq_indices, [&](auto i){
                constexpr auto irq = builder.irqs[i];
                return irq.template build<sub_value<BuilderValue, decltype(i)>>();
            });

            return hana::unpack(sub_irq_impls, [](auto ... sub_irq_impl_args) {
                return impl<decltype(sub_irq_impl_args)...>(sub_irq_impl_args...);
            });
        }

    private:
        template<typename... SubIrqImpls>
        struct impl {
        public:
            constexpr static auto irq_number = ConfigT::irq_number;

            /**
             * True if this shared_irq::impl has any active sub_irq::Impls, otherwise False.
             *
             * This is used to optimize compiled size and runtime performance. Unused irqs should
             * not consume any resources.
             */
            static bool constexpr active = (SubIrqImpls::active || ... || false);

        private:
            hana::tuple<SubIrqImpls...> sub_irq_impls;

        public:
            explicit constexpr impl(
                SubIrqImpls const & ... sub_irq_impls
            )
                :  sub_irq_impls(sub_irq_impls...)
            { }

            /**
             * Initialize and enable the hardware interrupt along with
             *
             * This should be used only by interrupt::Manager.
             *
             * @tparam InterruptHal
             *      The hardware abstraction layer that knows how to initialize hardware interrupts.
             */
            template<typename InterruptHal>
            inline void init_mcu_interrupts() const {
                // initialize the main irq hardware
                // TODO: unit test says MCU interrupts always get enabled for shared interrupts...why??
                enable_action<InterruptHal, true>();
            }

            auto get_interrupt_enables() const {
                if constexpr (active) {
                    auto const active_sub_irq_impls =
                        hana::filter(sub_irq_impls, [](auto irq) {
                            return hana::bool_c<decltype(irq)::active>;
                        });

                    return hana::unpack(active_sub_irq_impls, [](auto && ... irqs) {
                        return hana::flatten(hana::make_tuple(irqs.get_interrupt_enables()...));
                    });

                } else {
                    return hana::make_tuple();
                }
            }

            /**
             * Evaluate interrupt status of each sub_irq::impl and run each one with a pending
             * interrupt. Clear any hardware interrupt pending bits as necessary.
             *
             * This should be used only by interrupt::Manager.
             *
             * @tparam InterruptHal
             *      The hardware abstraction layer that knows how to clear pending interrupt status.
             */
            template<typename InterruptHal>
            inline void run() const {
                if constexpr (active) {
                    InterruptHal::template run<StatusPolicy>(irq_number, [&] {
                        hana::for_each(sub_irq_impls, [](auto irq) {
                            irq.run();
                        });
                    });
                }
            }
        };
    };




    /**
     * Type-erased interface to the interrupt manager.
     */
    class manager_interface {
    public:
        virtual void init() const = 0;
        virtual void init_mcu_interrupts() const = 0;
        virtual void init_sub_interrupts() const = 0;
    };




    /**
     * Declare one or more Irqs, SharedIrqs, and their corresponding interrupt service routine
     * attachment points.
     *
     * @tparam InterruptHal
     *      The hardware abstraction layer that knows how to clear pending interrupt status.
     *
     * @tparam IRQs
     */
    template<typename RootT>
    class manager {
    public:
        using InterruptHal = typename RootT::InterruptHal;

        constexpr static auto irqs_type =
            hana::transform(RootT::children, [](auto child){
                if constexpr (hana::size(child.children) > hana::size_c<0>) {
                    return shared_irq_builder<decltype(child)>{};
                } else {
                    return irq_builder<decltype(child)>{};
                }
            });

        std::remove_cv_t<decltype(irqs_type)> irqs;

        using Dynamic = dynamic_controller<RootT>;

    public:
        /**
         * Add interrupt service routine(s) to be executed when this IRQ is triggered.
         *
         * @tparam IrqType
         *      The IrqType the flow_description should be attached to.
         * 
         * @param flow_description
         *      See flow::Builder<>.add()
         */
        template<typename IrqType, typename T>
        void constexpr add(T const & flow_description) {
            hana::for_each(irqs, [&](auto & irq) {
                irq.template add<IrqType>(flow_description);
            });
        }

        /**
         * @return Manager::impl specialization optimized for size and runtime.
         */
        template<typename SizeTupleTupleIntConst>
        [[nodiscard]] auto constexpr internal_build() const {
            auto constexpr flow_sizes = SizeTupleTupleIntConst{};
            auto const size_with_irq = hana::zip(flow_sizes, irqs);

            auto const irq_impls = hana::transform(size_with_irq, [](auto size_irq_pair){
                return hana::unpack(size_irq_pair, [](auto size, auto irq){
                    return irq.template build<decltype(size)>();
                });
            });

            return hana::unpack(irq_impls, [](auto ... irq_impl_args) {
                return impl<decltype(irq_impl_args)...>(irq_impl_args...);
            });
        }

        /**
         * Created by calling Manager.build().
         *
         * Manager::impl is the runtime component of Manager. It is responsible for initializing
         * and running interrupts while using the least amount of run time, instruction memory,
         * and data memory. It will only initialize interrupts that have interrupt service routines
         * associated with them. If any irq is unused, it will not even generate any code for the
         * unused irqs.
         *
         * @tparam IrqImplTypes
         *      irq and shared_irq implementations. These are created by calling build() on each
         *      of the irq and shared_irq instances from within Manager.
         */
        template<typename... IrqImplTypes>
        class impl : public manager_interface {
        private:
            hana::tuple<IrqImplTypes...> irq_impls;

        public:
            explicit constexpr impl(
                IrqImplTypes... irq_impls
            )
                : irq_impls{irq_impls...}
            { }

            /**
             * Initialize the interrupt hardware and each of the active irqs.
             */
            virtual void init() const override final {
                // TODO: log exact interrupt manager configuration
                //       (should be a single compile-time string with no arguments)
                init_mcu_interrupts();
                init_sub_interrupts();
            }

            /**
             * Initialize the interrupt hardware and each of the active irqs.
             */
            virtual void init_mcu_interrupts() const override final {
                InterruptHal::init();
                hana::for_each(irq_impls, [](auto irq){
                    irq.template init_mcu_interrupts<InterruptHal>();
                });
            }

            /**
             * Initialize the interrupt hardware and each of the active irqs.
             */
            virtual void init_sub_interrupts() const override final {
                auto const interrupt_enables_tuple =
                    hana::unpack(irq_impls, [](auto... irqs_pack) {
                        return hana::flatten(hana::make_tuple(irqs_pack.get_interrupt_enables()...));
                    });

                hana::unpack(interrupt_enables_tuple, [](auto... interrupt_enables){
                    Dynamic::template enable_by_field<true, decltype(interrupt_enables)...>();
                });
            }

            /**
             * Execute the given IRQ number.
             *
             * The microcontroller's interrupt vector table should be configured to call this
             * method for each IRQ it supports.
             *
             * @tparam IrqNumber
             *      The IRQ number that has been triggered by hardware.
             */
            template<std::size_t IrqNumber>
            inline void run() const {
                // find the IRQ with the matching number
                auto const irq = hana::find_if(irq_impls, [](auto i){
                    return hana::bool_c<IrqNumber == decltype(i)::irq_number>;
                });

                auto constexpr run_irq = [](auto & irq){
                    irq.template run<InterruptHal>();
                    return true;
                };

                // if the IRQ was found, then run it, otherwise do nothing
                hana::maybe(false, run_irq, irq);
            }

            /**
             * @return The highest active IRQ number.
             */
            [[nodiscard]] std::size_t constexpr max_irq() const {
                auto const irq_numbers = hana::transform(irq_impls, [](auto irq){
                    return decltype(irq)::irq_number;
                });

                return hana::maximum(irq_numbers);
            }
        };

        ///////////////////////////////////////////////////////////////////////////
        ///
        /// Everything below is for the cib extension interface. It lets cib know
        /// this builder supports the cib pattern and how to build it.
        ///
        ///////////////////////////////////////////////////////////////////////////
        /**
         * Never called, but the return type is used by cib to determine what the
         * abstract interface is.
         */
        manager_interface * base() const;

        template<
            typename BuilderValue,
            typename Index>
        struct sub_value {
            constexpr static auto const & value = BuilderValue::value.irqs[Index{}];
        };

        /**
         * Given a constexpr Manager instance stored in BuilderValue::value, build
         * an optimal Manager::impl instance to initialize and run interrupts at runtime.
         *
         * @tparam BuilderValue
         *      A type with a static constexpr Manager field to be built into a Manager::impl
         *
         * @return The optimized Manager::impl to be used at runtime.
         */
        template<typename BuilderValue>
        [[nodiscard]] static auto constexpr build() {
            auto constexpr builder = BuilderValue::value;

            auto constexpr irq_indices =
                hana::to<hana::tuple_tag>(hana::make_range(hana::int_c<0>, hana::size(builder.irqs)));

            auto const irq_impls = hana::transform(irq_indices, [&](auto i){
                constexpr auto irq = builder.irqs[i];
                return irq.template build<sub_value<BuilderValue, decltype(i)>>();
            });

            return hana::unpack(irq_impls, [](auto ... irq_impl_args) {
                return impl<decltype(irq_impl_args)...>(irq_impl_args...);
            });
        }
    };
 }