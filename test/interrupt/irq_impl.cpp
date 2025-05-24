#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
using no_flows_config_t = interrupt::irq<42_irq, 17, interrupt::policies<>>;
}

TEST_CASE("config models concept", "[irq_impl]") {
    STATIC_REQUIRE(interrupt::irq_config<no_flows_config_t>);
}

TEST_CASE("config can enable/disable its irq", "[irq_impl]") {
    enabled<42_irq> = false;
    no_flows_config_t::enable<true>();
    CHECK(enabled<42_irq>);
    no_flows_config_t::enable<false>();
    CHECK(not enabled<42_irq>);
}

TEST_CASE("config enables its irq with priority", "[irq_impl]") {
    priority<42_irq> = 0;
    no_flows_config_t::enable<true>();
    CHECK(priority<42_irq> == 17);
}

TEST_CASE("config default status policy is clear first", "[irq_impl]") {
    STATIC_REQUIRE(std::is_same_v<no_flows_config_t::status_policy_t,
                                  interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[irq_impl]") {
    using config_t =
        interrupt::irq<42_irq, 17,
                       interrupt::policies<interrupt::clear_status_last>>;
    STATIC_REQUIRE(std::is_same_v<config_t::status_policy_t,
                                  interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[irq_impl]") {
    using impl_t = interrupt::irq_impl<no_flows_config_t, test_nexus>;
    STATIC_REQUIRE(interrupt::irq_interface<impl_t>);
}

namespace {
template <typename T>
using flow_config_t = interrupt::irq<17_irq, 42, interrupt::policies<>, T>;
} // namespace

TEST_CASE("impl runs a flow", "[irq_impl]") {
    using impl_t =
        interrupt::irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_REQUIRE(impl_t::active);
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl can init its interrupt", "[irq_impl]") {
    using impl_t =
        interrupt::irq_impl<flow_config_t<std::true_type>, test_nexus>;
    enabled<17_irq> = false;
    priority<17_irq> = 0;
    impl_t::init_mcu_interrupts();
    CHECK(enabled<17_irq>);
    CHECK(priority<17_irq> == 42);
}

TEST_CASE("impl is inactive when flow is not active", "[irq_impl]") {
    using impl_t =
        interrupt::irq_impl<flow_config_t<std::false_type>, test_nexus>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl is inactive when there are no flows", "[irq_impl]") {
    using impl_t = interrupt::irq_impl<no_flows_config_t, test_nexus>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl has no enable fields", "[irq_impl]") {
    using impl_t = interrupt::irq_impl<no_flows_config_t, test_nexus>;
    CHECK(impl_t::get_interrupt_enables() == stdx::tuple{});
}
