#pragma once

#include <interrupt/concepts.hpp>

#include <stdx/bitset.hpp>
#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <conc/concurrency.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/utility.hpp>

#include <type_traits>
#include <utility>

namespace interrupt {
namespace detail {
template <typename Irq> using get_resources_t = typename Irq::resources_t;
template <typename Irq>
using get_all_resources_t = typename Irq::all_resources_t;

template <typename Irq> using get_flows_t = typename Irq::flows_t;
template <typename Irq> using get_all_flows_t = typename Irq::all_flows_t;

template <typename Irq>
using get_name_list_t = stdx::type_list<typename Irq::name_t>;

template <typename Irq>
using has_real_enable_field =
    std::bool_constant<not is_no_field_v<typename Irq::enable_field_t>>;

template <typename Irq>
using has_enable_field = boost::mp11::mp_eval_if_not<
    std::bool_constant<requires { typename Irq::enable_field_t; }>,
    std::false_type, has_real_enable_field, Irq>;

template <typename Irq>
using enable_fields_t =
    boost::mp11::mp_remove_if<stdx::type_list<typename Irq::enable_field_t>,
                              is_no_field>;

template <typename Irq>
using get_enable_field_t =
    boost::mp11::mp_eval_if_not<has_enable_field<Irq>, stdx::type_list<>,
                                enable_fields_t, Irq>;

template <typename Root>
using descendants_t = std::remove_cvref_t<decltype(Root::descendants)>;

template <typename Root, template <typename...> typename F,
          template <typename...> typename Result = stdx::type_bitset,
          template <typename> typename Desc = descendants_t>
using collect_t = boost::mp11::mp_apply<
    Result,
    boost::mp11::mp_unique<boost::mp11::mp_apply<
        boost::mp11::mp_append, boost::mp11::mp_transform<F, Desc<Root>>>>>;

template <typename Name> struct has_name {
    template <typename Irq> using fn = std::is_same<Name, typename Irq::name_t>;
};

template <typename Irq> using get_name_t = typename Irq::name_t;

template <typename Item, template <typename> typename F> struct has_item_by {
    template <typename Irq> using fn = boost::mp11::mp_contains<F<Irq>, Item>;
};

template <typename Irqs> struct with_resource {
    template <typename Resource>
    using fn = stdx::tt_pair<
        Resource,
        boost::mp11::mp_transform<
            get_name_t, boost::mp11::mp_copy_if_q<
                            Irqs, has_item_by<Resource, get_resources_t>>>>;
};

template <typename Irqs> struct with_resource_propagated {
    template <typename Resource>
    using fn = stdx::tt_pair<
        Resource,
        boost::mp11::mp_transform<
            get_name_t, boost::mp11::mp_copy_if_q<
                            Irqs, has_item_by<Resource, get_all_resources_t>>>>;
};

template <typename Irqs> struct with_flow {
    template <typename Flow>
    using fn = stdx::tt_pair<
        Flow,
        boost::mp11::mp_transform<
            get_name_t,
            boost::mp11::mp_copy_if_q<Irqs, has_item_by<Flow, get_flows_t>>>>;
};

template <typename Irqs> struct with_flow_propagated {
    template <typename Flow>
    using fn = stdx::tt_pair<
        Flow, boost::mp11::mp_transform<
                  get_name_t, boost::mp11::mp_copy_if_q<
                                  Irqs, has_item_by<Flow, get_all_flows_t>>>>;
};

template <typename Field> struct register_for {
    template <typename Hal> constexpr static auto fn() {
        return Hal::template get_register<Field>();
    }
};
template <> struct register_for<no_field_t> {
    template <typename Hal> constexpr static auto fn() {
        return no_register_t{};
    }
};

template <typename Hal> struct get_register_q {
    template <typename Field>
    using from_field = decltype(register_for<Field>::template fn<Hal>());

    template <typename Irq>
    using from_irq =
        decltype(register_for<typename Irq::enable_field_t>::template fn<
                 Hal>());
};

namespace dynamic_hal_concept {
template <typename Cfg>
using enable_fields_t = collect_t<Cfg, get_enable_field_t>;

template <typename Cfg>
using first_enable_field_t = boost::mp11::mp_first<enable_fields_t<Cfg>>;

template <typename Hal, typename Cfg>
using register_type_t =
    decltype(register_for<first_enable_field_t<Cfg>>::template fn<Hal>());

template <typename Hal, typename Cfg>
using register_datatype_t =
    typename Hal::template register_datatype_t<register_type_t<Hal, Cfg>>;
} // namespace dynamic_hal_concept

template <typename Hal, typename Cfg>
concept dynamic_hal_for =
    boost::mp11::mp_empty<dynamic_hal_concept::enable_fields_t<Cfg>>::value or
    requires(dynamic_hal_concept::register_datatype_t<Hal, Cfg> value) {
        typename dynamic_hal_concept::register_type_t<Hal, Cfg>;
        {
            Hal::template mask<dynamic_hal_concept::register_type_t<Hal, Cfg>,
                               dynamic_hal_concept::first_enable_field_t<Cfg>>
        } -> stdx::same_as_unqualified<
            dynamic_hal_concept::register_datatype_t<Hal, Cfg>>;
        Hal::write(
            register_for<dynamic_hal_concept::first_enable_field_t<Cfg>>::
                template fn<Hal>(),
            value);
    };

template <typename Cfg, typename Rsrcs>
[[nodiscard]] auto on_by_resource(Rsrcs const &resource_enables) -> bool {
    constexpr auto resources_bs = Rsrcs{typename Cfg::resources_t{}};
    if constexpr (resources_bs.none() and
                  Cfg::resource_children_t::size() > 0) {
        return stdx::any_of(
            [&]<typename Child>(Child) {
                return on_by_resource<Child>(resource_enables);
            },
            typename Cfg::resource_children_t{});
    }
    return (resource_enables & resources_bs) == resources_bs;
}

struct propagate_t {
    template <typename Irqs, typename Resources>
    using resource_map_t = boost::mp11::mp_apply<
        stdx::type_map, boost::mp11::mp_transform_q<
                            detail::with_resource_propagated<Irqs>, Resources>>;

    template <typename Irqs, typename Flows>
    using flow_map_t = boost::mp11::mp_apply<
        stdx::type_map,
        boost::mp11::mp_transform_q<detail::with_flow_propagated<Irqs>, Flows>>;

    template <typename Irq> using resources_t = get_all_resources_t<Irq>;
    template <typename Irq> using flows_t = get_all_flows_t<Irq>;
};

struct no_propagate_t {
    template <typename Irqs, typename Resources>
    using resource_map_t = boost::mp11::mp_apply<
        stdx::type_map,
        boost::mp11::mp_transform_q<detail::with_resource<Irqs>, Resources>>;

    template <typename Irqs, typename Flows>
    using flow_map_t = boost::mp11::mp_apply<
        stdx::type_map,
        boost::mp11::mp_transform_q<detail::with_flow<Irqs>, Flows>>;

    template <typename Irq> using resources_t = get_resources_t<Irq>;
    template <typename Irq> using flows_t = get_flows_t<Irq>;
};
} // namespace detail

// Whether to propagate resource/flow enables & disables
// Note: it only makes sense to propagate up: children will not be run anyway if
// their parents are not run, so disabling a parent automatically disables
// children
constexpr inline auto propagate = detail::propagate_t{};
constexpr inline auto no_propagate = detail::no_propagate_t{};

template <typename Root, detail::dynamic_hal_for<Root> Hal>
struct dynamic_controller {
  private:
    // keep track of which resources/flows/irqs have been manually
    // enabled/disabled
    using resources_t = detail::collect_t<Root, detail::get_resources_t>;
    constinit static inline resources_t resource_enables{};

    using flows_t = detail::collect_t<Root, detail::get_flows_t>;
    constinit static inline flows_t flow_enables{};

    using irq_names_t = detail::collect_t<Root, detail::get_name_list_t>;
    constinit static inline irq_names_t named_enables{};

    // update bitsets as necessary
    template <typename T> static auto enable_one() -> void {
        if constexpr (boost::mp11::mp_contains<resources_t, T>::value) {
            resource_enables.template set<T>();
        } else if constexpr (boost::mp11::mp_contains<flows_t, T>::value) {
            flow_enables.template set<T>();
        } else {
            named_enables.template set<T>();
        }
    }

    template <typename T> static auto disable_one() -> void {
        if constexpr (boost::mp11::mp_contains<resources_t, T>::value) {
            resource_enables.template reset<T>();
        } else if constexpr (boost::mp11::mp_contains<flows_t, T>::value) {
            flow_enables.template reset<T>();
        } else {
            named_enables.template reset<T>();
        }
    }

    using irqs_t = detail::collect_t<Root, stdx::type_list>;

    // cached values for each enable register
    template <typename Register>
    using register_t = typename Hal::template register_datatype_t<Register>;

    template <typename Register>
    constinit static inline register_t<Register> cached_enable{};

    // which IRQs are potentially affected by a change?
    template <typename PropagatePolicy, typename... Ts>
    consteval static auto compute_affected_irqs() {
        // resource and flow influences are according to propagation policy
        using resource_map_t =
            PropagatePolicy::template resource_map_t<irqs_t, resources_t>;
        using flow_map_t =
            PropagatePolicy::template flow_map_t<irqs_t, flows_t>;

        using affected_irq_names_by_resource_t = boost::mp11::mp_append<
            stdx::type_lookup_t<resource_map_t, Ts, stdx::type_list<>>...>;
        using affected_irq_names_by_flow_t = boost::mp11::mp_append<
            stdx::type_lookup_t<flow_map_t, Ts, stdx::type_list<>>...>;

        using affected_irqs_t =
            decltype(stdx::apply_sequence<
                     boost::mp11::mp_unique<boost::mp11::mp_append<
                         affected_irq_names_by_resource_t,
                         affected_irq_names_by_flow_t>>>([]<typename... Ss>() {
                return boost::mp11::mp_append<boost::mp11::mp_copy_if_q<
                    detail::descendants_t<Root>, detail::has_name<Ss>>...>{};
            }));
        return affected_irqs_t{};
    }

    // compute changing value/mask for register
    template <typename Reg, typename Irq, typename T>
    static auto compute_register(T &new_value, T &mask) -> void {
        constexpr auto field_mask =
            Hal::template mask<Reg, typename Irq::enable_field_t>;
        mask |= field_mask;

        auto const on_by_flows = [] {
            constexpr auto flows_bs = flows_t{typename Irq::all_flows_t{}};
            if constexpr (flows_bs.none()) {
                return true;
            }
            return (flow_enables & flows_bs) != flows_t{};
        }();

        if (detail::on_by_resource<Irq>(resource_enables) and on_by_flows and
            named_enables[stdx::type_identity_v<typename Irq::name_t>]) {
            new_value |= field_mask;
        }
    }

    // update cached values and write the changed registers
    template <template <typename...> typename L, typename... Irqs>
    static auto update(L<Irqs...>) -> void {
        constexpr auto by_registers =
            stdx::gather_by<detail::get_register_q<Hal>::template from_irq>(
                stdx::tuple<Irqs...>{});

        stdx::for_each(
            []<typename I, typename... Is>(stdx::tuple<I, Is...>) -> void {
                auto r = detail::register_for<
                    typename I::enable_field_t>::template fn<Hal>();
                if constexpr (not is_no_register_v<decltype(r)>) {

                    using reg_t = decltype(r);
                    auto mask = register_t<reg_t>{};
                    auto value = register_t<reg_t>{};
                    (compute_register<reg_t, I>(value, mask), ...,
                     compute_register<reg_t, Is>(value, mask));

                    auto &cached_value = cached_enable<reg_t>;
                    auto new_value = (cached_value & ~mask) | value;
                    if (new_value != std::exchange(cached_value, new_value)) {
                        Hal::write(r, cached_value);
                    }
                }
            },
            by_registers);
    }

    // reset: mark all resources, flows and named irqs enabled
    // and clear all cached state
    template <bool Enable, typename IrqList>
    static auto reset_internal_state() -> void {
        if constexpr (Enable) {
            resource_enables = resources_t{stdx::all_bits};
            flow_enables = flows_t{stdx::all_bits};
            named_enables = irq_names_t{stdx::all_bits};
        } else {
            resource_enables.reset();
            flow_enables.reset();
            named_enables.reset();
        }

        constexpr auto by_registers =
            stdx::gather_by<detail::get_register_q<Hal>::template from_irq>(
                IrqList{});
        stdx::for_each(
            []<typename I, typename... Is>(stdx::tuple<I, Is...>) -> void {
                using reg_t =
                    decltype(detail::register_for<
                             typename I::enable_field_t>::template fn<Hal>());
                if constexpr (not is_no_register_v<reg_t>) {
                    cached_enable<reg_t> = register_t<reg_t>{};
                    if constexpr (not Enable) {
                        auto value = register_t<reg_t>{};
                        auto &mask = cached_enable<reg_t>;
                        (compute_register<reg_t, I>(value, mask), ...,
                         compute_register<reg_t, Is>(value, mask));
                    }
                }
            },
            by_registers);
    }

  public:
    using hal_t = Hal;
    struct mutex_t;

    template <bool Enable = true> static auto init() -> void {
        using enable_irqs_t =
            boost::mp11::mp_copy_if<detail::descendants_t<Root>,
                                    detail::has_enable_field>;
        conc::call_in_critical_section<mutex_t>([] {
            reset_internal_state<Enable, enable_irqs_t>();
            update(enable_irqs_t{});
        });
    }

    template <typename... Ts, typename PropagatePolicy = detail::propagate_t>
    static auto enable(PropagatePolicy = {}) -> void {
        conc::call_in_critical_section<mutex_t>([] {
            constexpr auto affected_irqs =
                compute_affected_irqs<PropagatePolicy, Ts...>();
            (enable_one<Ts>(), ...);
            update(affected_irqs);
        });
    }
    template <typename... Ts, typename PropagatePolicy = detail::propagate_t>
    static auto disable(PropagatePolicy = {}) -> void {
        conc::call_in_critical_section<mutex_t>([] {
            constexpr auto affected_irqs =
                compute_affected_irqs<PropagatePolicy, Ts...>();
            (disable_one<Ts>(), ...);
            update(affected_irqs);
        });
    }

    template <stdx::ct_string... IrqNames> static auto enable() -> void {
        using affected_irqs_t =
            boost::mp11::mp_append<boost::mp11::mp_copy_if_q<
                detail::descendants_t<Root>,
                detail::has_name<stdx::cts_t<IrqNames>>>...>;
        conc::call_in_critical_section<mutex_t>([] {
            (enable_one<stdx::cts_t<IrqNames>>(), ...);
            update(affected_irqs_t{});
        });
    }
    template <stdx::ct_string... IrqNames> static auto disable() -> void {
        using affected_irqs_t =
            boost::mp11::mp_append<boost::mp11::mp_copy_if_q<
                detail::descendants_t<Root>,
                detail::has_name<stdx::cts_t<IrqNames>>>...>;
        conc::call_in_critical_section<mutex_t>([] {
            (disable_one<stdx::cts_t<IrqNames>>(), ...);
            update(affected_irqs_t{});
        });
    }
};
} // namespace interrupt
