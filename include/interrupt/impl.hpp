#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/hal.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace interrupt {
template <typename Nexus, typename Config>
concept nexus_for_cfg = Config::template has_flows_for<Nexus>;

template <typename Config, nexus_for_cfg<Config>... Nexi>
struct irq_impl : Config {
    constexpr static bool active = Config::template active<Nexi...>;

    static auto init_mcu_interrupts() -> void {
        Config::template enable<active>();
    }

    [[nodiscard]] static auto get_interrupt_enables() -> stdx::tuple<> {
        return {};
    }

    static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            hal::run<status_policy_t>(Config::irq_number,
                                      [] { Config::template isr<Nexi...>(); });
        }
    }
};

template <typename Config, nexus_for_cfg<Config>... Nexi>
struct sub_irq_impl : Config {
    constexpr static bool active = Config::template active<Nexi...>;

    using Config::enable_field;
    using Config::status_field;

    [[nodiscard]] static auto get_interrupt_enables() {
        if constexpr (active) {
            return stdx::make_tuple(enable_field);
        } else {
            return stdx::tuple{};
        }
    }

    static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            if (apply(read(enable_field)) && apply(read(status_field))) {
                status_policy_t::run([&] { apply(clear(status_field)); },
                                     [&] { Config::template isr<Nexi...>(); });
            }
        }
    }
};

template <typename Config, sub_irq_interface... Subs>
struct shared_irq_impl : Config {
    constexpr static bool active = (Subs::active or ...);

    static auto init_mcu_interrupts() -> void {
        Config::template enable<active>();
    }

    [[nodiscard]] static auto get_interrupt_enables() {
        return stdx::tuple_cat(Subs::get_interrupt_enables()...);
    }

    static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            hal::run<status_policy_t>(Config::irq_number,
                                      [] { (Subs::run(), ...); });
        }
    }
};

template <typename Config, sub_irq_interface... Subs>
struct shared_sub_irq_impl : Config {
    constexpr static bool active = (Subs::active or ...);

    using Config::enable_field;
    using Config::status_field;

    [[nodiscard]] static auto get_interrupt_enables() {
        return stdx::tuple_cat(Subs::get_interrupt_enables()...);
    }

    static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            if (apply(read(enable_field)) && apply(read(status_field))) {
                status_policy_t::run([&] { apply(clear(status_field)); },
                                     [&] { (Subs::run(), ...); });
            }
        }
    }
};
} // namespace interrupt
