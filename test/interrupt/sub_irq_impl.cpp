#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
using no_flows_config_t =
    interrupt::sub_irq<enable_field_t<0>, status_field_t<0>,
                       interrupt::policies<>>;
} // namespace

TEST_CASE("config models concept", "[sub_irq_impl]") {
    STATIC_REQUIRE(interrupt::sub_irq_config<no_flows_config_t>);
}

TEST_CASE("config default status policy is clear first", "[sub_irq_impl]") {
    STATIC_REQUIRE(std::is_same_v<no_flows_config_t::status_policy_t,
                                  interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[sub_irq_impl]") {
    using config_t =
        interrupt::sub_irq<enable_field_t<0>, status_field_t<0>,
                           interrupt::policies<interrupt::clear_status_last>>;
    STATIC_REQUIRE(std::is_same_v<config_t::status_policy_t,
                                  interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[sub_irq_impl]") {
    using impl_t = interrupt::sub_irq_impl<no_flows_config_t, test_nexus>;
    STATIC_REQUIRE(interrupt::sub_irq_interface<impl_t>);
}

namespace {
template <typename T>
using flow_config_t = interrupt::sub_irq<enable_field_t<0>, status_field_t<0>,
                                         interrupt::policies<>, T>;
}

TEST_CASE("impl runs a flow when enabled and status", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_REQUIRE(impl_t::active);

    enable_field_t<0>::value = true;
    status_field_t<0>::value = true;
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl doesn't run a flow when not enabled", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_REQUIRE(impl_t::active);

    enable_field_t<0>::value = false;
    status_field_t<0>::value = true;
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(not flow_run<std::true_type>);
}

TEST_CASE("impl doesn't run a flow when not status", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_REQUIRE(impl_t::active);

    enable_field_t<0>::value = true;
    status_field_t<0>::value = false;
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(not flow_run<std::true_type>);
}

TEST_CASE("impl is inactive when flow is not active", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::false_type>, test_nexus>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl is inactive when there are no flows", "[sub_irq_impl]") {
    using impl_t = interrupt::sub_irq_impl<no_flows_config_t, test_nexus>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl reports one enable field when active", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    CHECK(impl_t::get_interrupt_enables() == stdx::tuple{enable_field_t<0>{}});
}

TEST_CASE("impl reports no enable fields when not active", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::false_type>, test_nexus>;
    CHECK(impl_t::get_interrupt_enables() == stdx::tuple{});
}

TEST_CASE("impl reports no enable fields when there are no flows",
          "[sub_irq_impl]") {
    using impl_t = interrupt::sub_irq_impl<no_flows_config_t, test_nexus>;
    CHECK(impl_t::get_interrupt_enables() == stdx::tuple{});
}
