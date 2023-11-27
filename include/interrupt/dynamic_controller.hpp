#pragma once

#include <conc/concurrency.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <limits>
#include <type_traits>

namespace interrupt {
enum class resource_status { OFF = 0, ON = 1 };

template <typename Irq>
concept has_enable_field = requires { Irq::enable_field; };

template <typename Irq>
concept has_resource =
    has_enable_field<Irq> and
    not boost::mp11::mp_empty<typename Irq::resources_t>::value;

template <typename Root> struct dynamic_controller {
  private:
    using all_resources_t =
        boost::mp11::mp_unique<decltype(Root::descendants.apply(
            []<typename... Irqs>(Irqs const &...) {
                return boost::mp11::mp_append<typename Irqs::resources_t...>{};
            }))>;

    template <typename Register>
    CONSTINIT static inline typename Register::DataType allowed_enables =
        std::numeric_limits<typename Register::DataType>::max();

    template <typename Register>
    CONSTINIT static inline typename Register::DataType dynamic_enables{};

    template <typename Resource>
    CONSTINIT static inline bool is_resource_on = true;

    template <typename Resource> struct doesnt_require_resource {
        template <typename Irq>
        using fn =
            std::bool_constant<has_enable_field<Irq> and
                               not boost::mp11::mp_contains<
                                   typename Irq::resources_t, Resource>::value>;
    };

    template <typename Register> struct in_register {
        template <typename Field>
        using fn = std::is_same<Register, typename Field::RegisterType>;
    };

    /**
     * For each ResourceType, keep track of what interrupts can still be enabled
     * when that resource goes down.
     *
     * Each bit in this mask corresponds to an interrupt enable field in
     * RegType. If the bit is '1', that means the corresponding interrupt can be
     * enabled when the resource is not available. If the bit is '0', that means
     * the corresponding interrupt must be disabled when the resource is not
     * available.
     *
     * @tparam ResourceType
     *      The resource we want to check.
     *
     * @tparam RegType
     *      The specific register mask we want to check.
     */
    template <typename ResourceType, typename RegType>
    constexpr static typename RegType::DataType irqs_allowed = []() {
        // get all interrupt enable fields that don't require the given resource
        auto const matching_irqs =
            stdx::filter<doesnt_require_resource<ResourceType>::template fn>(
                Root::descendants);
        auto const interrupt_enables_tuple = stdx::transform(
            [](auto irq) { return irq.enable_field; }, matching_irqs);

        // filter fields that aren't in RegType
        auto const fields_in_reg =
            stdx::filter<in_register<RegType>::template fn>(
                interrupt_enables_tuple);

        // set the bits in the mask for interrupts that don't require the
        // resource
        using DataType = typename RegType::DataType;
        return fields_in_reg.fold_left(
            DataType{}, [](DataType value, auto field) -> DataType {
                return value | field.get_mask();
            });
    }();

    template <typename RegTypeTuple>
    static inline void reprogram_interrupt_enables(RegTypeTuple regs) {
        stdx::for_each(
            []<typename R>(R reg) {
                // make sure we don't enable any interrupts that are not allowed
                // according to resource availability
                auto const final_enables =
                    allowed_enables<R> & dynamic_enables<R>;

                // update the hardware registers
                apply(write(reg.raw(final_enables)));
            },
            regs);
    }

    /**
     * tuple of every interrupt register affected by a resource
     */
    template <typename Irq>
    using has_resource_t = std::bool_constant<has_resource<Irq>>;

    constexpr static auto all_resource_affected_regs =
        stdx::to_unsorted_set(stdx::transform(
            []<typename Irq>(Irq) { return Irq::enable_field.get_register(); },
            stdx::filter<has_resource_t>(Root::descendants)));

    /**
     * Reprogram interrupt enables based on updated resource availability.
     */
    static inline auto recalculate_allowed_enables() {
        // set allowed_enables mask for each resource affected register
        stdx::for_each(
            []<typename R>(R) {
                using DataType = typename R::DataType;
                allowed_enables<R> = std::numeric_limits<DataType>::max();
            },
            all_resource_affected_regs);

        // for each resource, if it is not on, mask out unavailable interrupts
        stdx::template_for_each<all_resources_t>([]<typename Rsrc>() {
            if (not is_resource_on<Rsrc>) {
                stdx::for_each(
                    []<typename R>(R) {
                        allowed_enables<R> &= irqs_allowed<Rsrc, R>;
                    },
                    all_resource_affected_regs);
            }
        });

        return all_resource_affected_regs;
    }

    /**
     * Store the interrupt enable values that FW _wants_ at runtime,
     * irrespective of any resource conflicts that would require specific
     * interrupts to be disabled.
     *
     * @tparam RegType
     *      The croo::Register this value corresponds to.
     */

    template <typename... Flows> struct match_flow {
        template <typename Irq>
        using fn =
            std::bool_constant<has_enable_field<Irq> and
                               (... or Irq::template triggers_flow<Flows>)>;
    };

    template <bool Enable, typename... Flows>
    static inline void enable_by_name() {
        // NOTE: critical section is not needed here because shared state is
        // only updated by the final call to enable_by_field

        // TODO: add support to enable/disable top-level IRQs by name.
        //       this will require another way to manage them vs. mmio
        //       registers. once that goes in, then enable_by_field should be
        //       removed or made private.
        auto const matching_irqs =
            stdx::filter<match_flow<Flows...>::template fn>(Root::descendants);

        auto const interrupt_enables_tuple = stdx::transform(
            [](auto irq) { return irq.enable_field; }, matching_irqs);

        interrupt_enables_tuple.apply([]<typename... Fields>(Fields...) {
            enable_by_field<Enable, Fields...>();
        });
    }

    template <typename ResourceType>
    static inline void update_resource(resource_status status) {
        conc::call_in_critical_section<dynamic_controller>([&] {
            is_resource_on<ResourceType> = (status == resource_status::ON);
            recalculate_allowed_enables();
            reprogram_interrupt_enables(all_resource_affected_regs);
        });
    }

  public:
    template <typename ResourceType> static inline void turn_on_resource() {
        update_resource<ResourceType>(resource_status::ON);
    }

    template <typename ResourceType> static inline void turn_off_resource() {
        update_resource<ResourceType>(resource_status::OFF);
    }

    template <bool Enable, typename... Fields>
    static inline void enable_by_field() {
        conc::call_in_critical_section<dynamic_controller>([] {
            [[maybe_unused]] const auto enable = []<typename F>() -> void {
                using R = typename F::RegisterType;
                if constexpr (Enable) {
                    dynamic_enables<R> |= F::get_mask();
                } else {
                    dynamic_enables<R> &= ~F::get_mask();
                }
            };
            (enable.template operator()<Fields>(), ...);

            auto const unique_regs = stdx::to_unsorted_set(
                stdx::tuple<typename Fields::RegisterType...>{});
            reprogram_interrupt_enables(unique_regs);
        });
    }

    template <typename... Flows> static inline void enable() {
        enable_by_name<true, Flows...>();
    }

    template <typename... Flows> static inline void disable() {
        enable_by_name<false, Flows...>();
    }
};
} // namespace interrupt
