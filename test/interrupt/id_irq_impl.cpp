#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using config_t = interrupt::id_irq<"test", void, interrupt::policies<>>;
} // namespace

TEST_CASE("config models concept", "[id_irq_impl]") {
    STATIC_CHECK(interrupt::sub_irq_config<config_t>);
}

TEST_CASE("impl models concept", "[id_irq_impl]") {
    using impl_t = interrupt::id_irq_impl<config_t>;
    STATIC_CHECK(interrupt::sub_irq_interface<impl_t>);
}

TEST_CASE("impl can dump config", "[id_irq_impl]") {
    using namespace stdx::literals;
    using impl_t = interrupt::id_irq_impl<config_t>;
    constexpr auto s = impl_t::config();
    STATIC_CHECK(
        s == "interrupt::id_irq<\"test\", void, interrupt::policies<>>"_cts);
}
