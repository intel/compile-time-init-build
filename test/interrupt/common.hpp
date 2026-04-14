#pragma once

#include <interrupt/config.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/policies.hpp>

#include <groov/groov.hpp>
#include <groov/test.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

using interrupt::operator""_irq;

namespace {
template <typename interrupt::irq_num_t> bool enabled{};
template <typename interrupt::irq_num_t> std::size_t priority{};
bool inited{};

using namespace stdx::literals;
template <stdx::ct_string S> using en_field_t = stdx::cts_t<"enable."_cts + S>;
template <stdx::ct_string S> using st_field_t = stdx::cts_t<"status."_cts + S>;

template <typename Group> struct test_hal {
    static auto init() -> void { inited = true; }

    template <bool Enable, interrupt::irq_num_t IrqNumber, std::size_t Priority>
    static auto irq_init() -> void {
        enabled<IrqNumber> = Enable;
        priority<IrqNumber> = Priority;
    }

    template <typename Field>
    consteval static auto get_field() -> groov::pathlike auto {
        return groov::make_path<Field::value>();
    }
    template <typename Field>
    consteval static auto get_register() -> groov::pathlike auto {
        return groov::parent(get_field<Field>());
    }

    template <groov::pathlike Register>
    using register_datatype_t =
        typename decltype(groov::resolve(Group{}, Register{}))::type_t;

    template <groov::pathlike Register, typename Field>
    constexpr static register_datatype_t<Register> mask =
        groov::resolve(Group{}, groov::make_path<Field::value>())
            .template mask<register_datatype_t<Register>>;

    template <groov::pathlike P>
    static auto write(P p, auto raw_value) -> void {
        groov::sync_write(Group{}(p = raw_value));
    }
    template <groov::pathlike P> static auto read(P p) -> bool {
        auto const value = groov::test::get_value<Group>(groov::parent(p));
        REQUIRE(value);
        using Field = decltype(groov::resolve(Group{}, P{}));
        return Field::extract(*value);
    }
    template <groov::pathlike P> static auto clear(P p) -> void {
        groov::sync_write(Group{}(p = groov::clear));
    }
};
} // namespace

template <typename T> inline bool flow_run{};

template <typename T> struct flow_t {
    auto operator()() const { flow_run<T> = true; }
    constexpr static bool active{T::value};
};

struct test_nexus {
    template <typename T> constexpr static auto service = flow_t<T>{};
};
