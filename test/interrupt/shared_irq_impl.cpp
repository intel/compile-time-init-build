#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
using no_flows_config_t =
    interrupt::shared_irq<42_irq, 17, interrupt::policies<>>;
}

TEST_CASE("config models concept", "[shared_irq_impl]") {
    STATIC_REQUIRE(interrupt::irq_config<no_flows_config_t>);
}

TEST_CASE("config can enable/disable its irq", "[shared_irq_impl]") {
    enabled<42_irq> = false;
    no_flows_config_t::enable<true>();
    CHECK(enabled<42_irq>);
    no_flows_config_t::enable<false>();
    CHECK(not enabled<42_irq>);
}

TEST_CASE("config enables its irq with priority", "[shared_irq_impl]") {
    priority<42_irq> = 0;
    no_flows_config_t::enable<true>();
    CHECK(priority<42_irq> == 17);
}

TEST_CASE("config default status policy is clear first", "[shared_irq_impl]") {
    STATIC_REQUIRE(std::is_same_v<no_flows_config_t::status_policy_t,
                                  interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[shared_irq_impl]") {
    using config_t = interrupt::shared_irq<
        42_irq, 17, interrupt::policies<interrupt::clear_status_last>>;
    STATIC_REQUIRE(std::is_same_v<config_t::status_policy_t,
                                  interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[shared_irq_impl]") {
    using impl_t = interrupt::shared_irq_impl<no_flows_config_t>;
    STATIC_REQUIRE(interrupt::irq_interface<impl_t>);
}

namespace {
template <typename T, auto N>
using sub_config_t = interrupt::sub_irq<enable_field_t<N>, status_field_t<N>,
                                        interrupt::policies<>, T>;

template <typename T>
using flow_config_t = interrupt::shared_irq<17_irq, 42, interrupt::policies<>,
                                            sub_config_t<T, 0>>;
} // namespace

TEST_CASE("impl runs a flow", "[shared_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 0>, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;
    STATIC_REQUIRE(impl_t::active);

    enable_field_t<0>::value = true;
    status_field_t<0>::value = true;
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl can init its interrupt", "[shared_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 0>, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;

    enabled<17_irq> = false;
    priority<17_irq> = 0;
    impl_t::init_mcu_interrupts();
    CHECK(enabled<17_irq>);
    CHECK(priority<17_irq> == 42);
}

TEST_CASE("impl is inactive when all subs are inactive", "[shared_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 0>, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl is active when any sub is active", "[shared_irq_impl]") {
    using active_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 17>, test_nexus>;
    using inactive_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 42>, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>,
                                   active_sub_impl_t, inactive_sub_impl_t>;
    STATIC_REQUIRE(impl_t::active);
}

TEST_CASE("impl is inactive when there are no subs", "[shared_irq_impl]") {
    using impl_t = interrupt::shared_irq_impl<flow_config_t<std::true_type>>;
    STATIC_REQUIRE(not impl_t::active);
}

TEST_CASE("impl enable fields are active sub enable fields",
          "[shared_irq_impl]") {
    using active_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 17>, test_nexus>;
    using inactive_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 42>, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>,
                                   active_sub_impl_t, inactive_sub_impl_t>;
    CHECK(impl_t::get_interrupt_enables() == stdx::tuple{enable_field_t<17>{}});
}
