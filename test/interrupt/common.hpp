#pragma once

#include <interrupt/config.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>
#include <interrupt/policies.hpp>

#include <stdx/concepts.hpp>

#include <cstddef>

using interrupt::operator""_irq;

namespace {
template <typename interrupt::irq_num_t> bool enabled{};
template <typename interrupt::irq_num_t> std::size_t priority{};
bool inited{};

struct test_hal {
    static auto init() -> void { inited = true; }

    template <bool Enable, interrupt::irq_num_t IrqNumber, std::size_t Priority>
    static auto irq_init() -> void {
        enabled<IrqNumber> = Enable;
        priority<IrqNumber> = Priority;
    }

    template <interrupt::status_policy P>
    static auto run(interrupt::irq_num_t, stdx::invocable auto const &isr)
        -> void {
        P::run([] {}, [&] { isr(); });
    }
};
} // namespace
template <> inline auto interrupt::injected_hal<> = test_hal{};

template <typename T> inline bool flow_run{};

template <typename T> struct flow_t {
    auto operator()() const { flow_run<T> = true; }
    constexpr static bool active{T::value};
};

struct test_nexus {
    template <typename T> constexpr static auto service = flow_t<T>{};
};

template <auto> struct enable_field_t {
    static inline bool value{};
    friend constexpr auto operator==(enable_field_t, enable_field_t)
        -> bool = default;
};
template <auto> struct status_field_t {
    static inline bool value{};
};

template <typename Field> constexpr auto read(Field) {
    return [] { return Field::value; };
}
template <typename Field> constexpr auto clear(Field) {
    return [] { Field::value = false; };
}
template <typename... Ops> constexpr auto apply(Ops... ops) {
    return (ops(), ...);
}
