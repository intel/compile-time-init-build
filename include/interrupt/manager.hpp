#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/hal.hpp>

#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <type_traits>

namespace interrupt {
namespace detail {
template <typename Dynamic, irq_interface... Impls> struct manager {
    void init() const {
        // TODO: log exact interrupt manager configuration
        //       (should be a single compile-time string with no arguments)
        hal::init();
        init_mcu_interrupts();
        init_sub_interrupts();
    }

    void init_mcu_interrupts() const { (Impls::init_mcu_interrupts(), ...); }

    void init_sub_interrupts() const {
        auto enables = stdx::tuple_cat(Impls::get_interrupt_enables()...);
        enables.apply([]<typename... Enables>(Enables...) {
            Dynamic::template enable_by_field<true, Enables...>();
        });
    }

    template <irq_num_t Number> inline void run() const {
        using M = stdx::type_map<stdx::vt_pair<Impls::irq_number, Impls>...>;
        using irq_t = stdx::value_lookup_t<M, Number>;

        if constexpr (not std::is_void_v<irq_t>) {
            irq_t::run();
        }
    }

    [[nodiscard]] constexpr auto max_irq() const -> irq_num_t {
        return static_cast<irq_num_t>(
            std::max({stdx::to_underlying(Impls::irq_number)...}));
    }
};

template <typename Config> struct build_manager {
    using dynamic_t = typename Config::template dynamic_controller_t<Config>;
    template <typename... Built> using impl = manager<dynamic_t, Built...>;
};
} // namespace detail

template <interrupt::root_config Config, typename... Nexi>
using manager = typename Config::template build<
    detail::build_manager<Config>::template impl, Nexi...>;
} // namespace interrupt
