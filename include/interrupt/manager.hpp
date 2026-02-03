#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <type_traits>

namespace interrupt {
namespace detail {
template <typename Dynamic, irq_interface... Impls> struct manager {
    using hal_t = typename Dynamic::hal_t;
    using dynamic_t = Dynamic;

    void init() const {
        hal_t::init();
        (Impls::template init<hal_t>(), ...);
        dynamic_t::init();
    }

    constexpr static auto config() {
        return +stdx::ct_format<"interrupt::root<{}>">(
            detail::config_string_for<Impls...>());
    }

    template <irq_num_t Number> inline void run() const {
        using M = stdx::type_map<stdx::vt_pair<Impls::irq_number, Impls>...>;
        using irq_t = stdx::value_lookup_t<M, Number>;

        if constexpr (not std::is_void_v<irq_t>) {
            irq_t::template run<hal_t, typename dynamic_t::mutex_t>();
        }
    }

    [[nodiscard]] constexpr auto max_irq() const -> irq_num_t {
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
