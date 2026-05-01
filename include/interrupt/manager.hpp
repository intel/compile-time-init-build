#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/udls.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <type_traits>

namespace interrupt {
namespace detail {
template <typename Dynamic, irq_interface... Impls> struct manager {
    using hal_t = typename Dynamic::hal_t;
    using dynamic_t = Dynamic;

    template <bool DynamicEnableTopLevel = false> static auto init() -> void {
        init_mcu();
        init_top_level();
        init_dynamic<DynamicEnableTopLevel>();
    }

    static auto init_mcu() -> void { hal_t::init(); }

    static auto init_top_level() -> void {
        (Impls::template init<hal_t>(), ...);
    }

    template <bool DynamicEnableTopLevel = false>
    static auto init_dynamic() -> void {
        using namespace stdx::literals;
        using active_flows_t =
            boost::mp11::mp_append<typename Impls::active_flows_t...>;

        if constexpr (DynamicEnableTopLevel) {
            dynamic_t::template init<"enable"_true, active_flows_t,
                                     stdx::type_list<>>();
        } else {
            // if init_top_level also enabled the top level interrupts, we don't
            // want to rewrite the enables
            dynamic_t::template init<"enable"_true, active_flows_t,
                                     stdx::type_list<>, Impls...>();
        }
    }

    constexpr static auto config() {
        return +stdx::ct_format<"interrupt::root<{}>">(
            detail::config_string_for<Impls...>());
    }

    template <irq_num_t Number> static auto run() -> void {
        using M = stdx::type_map<stdx::vt_pair<Impls::irq_number, Impls>...>;
        using irq_t = stdx::value_lookup_t<M, Number>;

        if constexpr (not std::is_void_v<irq_t>) {
            irq_t::template run<hal_t, typename dynamic_t::mutex_t>();
        }
    }

    [[nodiscard]] constexpr static auto max_irq() -> irq_num_t {
        return static_cast<irq_num_t>(
            std::max({stdx::to_underlying(Impls::irq_number)...}));
    }
};

template <typename Config, typename Hal> struct build_manager {
    using dynamic_t =
        typename Config::template dynamic_controller_t<Config, Hal>;
    template <typename... Built> using impl = manager<dynamic_t, Built...>;
};
} // namespace detail

template <interrupt::root_config Config, typename Hal, typename... Nexi>
using manager = typename Config::template build<
    detail::build_manager<Config, Hal>::template impl, Nexi...>;
} // namespace interrupt
