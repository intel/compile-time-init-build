#pragma once

#include <interrupt/concepts.hpp>

#include <stdx/compiler.hpp>

#include <conc/concurrency.hpp>

namespace interrupt {

namespace detail {
template <typename Field> struct read_field {
    template <typename Hal> static auto with_hal() -> bool {
        using field_t = decltype(Hal::template get_field<Field>());
        return Hal::template read<field_t>();
    }
    template <typename Hal, typename Mutex> static auto with_hal() -> bool {
        return conc::call_in_critical_section<Mutex>(
            [] { return with_hal<Hal>(); });
    }
};

template <> struct read_field<no_field_t> {
    template <typename...> CONSTEVAL static auto with_hal() -> bool {
        return true;
    }
};

template <typename Field> struct clear_field {
    template <typename Hal> static auto with_hal() -> void {
        using field_t = decltype(Hal::template get_field<Field>());
        Hal::template clear<field_t>();
    }
    template <typename Hal, typename Mutex> static auto with_hal() -> void {
        conc::call_in_critical_section<Mutex>([] { with_hal<Hal>(); });
    }
};

template <> struct clear_field<no_field_t> {
    template <typename...> CONSTEVAL static auto with_hal() -> void {}
};
} // namespace detail

template <typename Nexus, typename Config>
concept nexus_for_cfg = Config::template has_flows_for<Nexus>;

template <typename Config, nexus_for_cfg<Config>... Nexi>
struct irq_impl : Config {
    constexpr static bool active = Config::template active<Nexi...>;

    template <typename Hal> static auto init() -> void {
        Config::template enable<active, Hal>();
    }

    template <typename Hal, typename = void> static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            Hal::template run<status_policy_t>(
                Config::irq_number, [] { Config::template isr<Nexi...>(); });
        }
    }
};

template <typename Config, nexus_for_cfg<Config>... Nexi>
struct sub_irq_impl : Config {
    constexpr static bool active = Config::template active<Nexi...>;

    template <typename Hal, typename Mutex = void> static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            using en_t = typename Config::enable_field_t;
            using st_t = typename Config::status_field_t;

            if (detail::read_field<en_t>::template with_hal<Hal, Mutex>() and
                detail::read_field<st_t>::template with_hal<Hal>()) {
                status_policy_t::run(
                    [&] {
                        detail::clear_field<st_t>::template with_hal<Hal>();
                    },
                    [&] { Config::template isr<Nexi...>(); });
            }
        }
    }
};

template <typename Config> struct id_irq_impl : Config {
    constexpr static bool active = true;

    template <typename...> static auto run() -> void {}
};

template <typename Config, sub_irq_interface... Subs>
struct shared_irq_impl : Config {
    constexpr static bool active = (Subs::active or ...);

    template <typename Hal> static auto init() -> void {
        Config::template enable<active, Hal>();
    }

    template <typename Hal, typename Mutex = void> static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            Hal::template run<status_policy_t>(Config::irq_number, [] {
                (Subs::template run<Hal, Mutex>(), ...);
            });
        }
    }
};

template <typename Config, sub_irq_interface... Subs>
struct shared_sub_irq_impl : Config {
    constexpr static bool active = (Subs::active or ...);

    template <typename Hal, typename Mutex = void> static auto run() -> void {
        if constexpr (active) {
            using status_policy_t = typename Config::status_policy_t;
            using en_t = typename Config::enable_field_t;
            using st_t = typename Config::status_field_t;

            if (detail::read_field<en_t>::template with_hal<Hal, Mutex>() and
                detail::read_field<st_t>::template with_hal<Hal>()) {
                status_policy_t::run(
                    [&] {
                        detail::clear_field<st_t>::template with_hal<Hal>();
                    },
                    [&] { (Subs::template run<Hal, Mutex>(), ...); });
            }
        }
    }
};
} // namespace interrupt
