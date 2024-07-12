#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
using no_flows_config_t =
    interrupt::shared_sub_irq<enable_field_t<0>, status_field_t<0>,
                              interrupt::policies<>>;
} // namespace

TEST_CASE("config models concept", "[shared_sub_irq_impl]") {
    static_assert(interrupt::sub_irq_config<no_flows_config_t>);
}

TEST_CASE("config default status policy is clear first",
          "[shared_sub_irq_impl]") {
    static_assert(std::is_same_v<no_flows_config_t::status_policy_t,
                                 interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[shared_sub_irq_impl]") {
    using config_t = interrupt::shared_sub_irq<
        enable_field_t<0>, status_field_t<0>,
        interrupt::policies<interrupt::clear_status_last>>;
    static_assert(std::is_same_v<config_t::status_policy_t,
                                 interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[shared_sub_irq_impl]") {
    using impl_t = interrupt::shared_sub_irq_impl<no_flows_config_t>;
    static_assert(interrupt::sub_irq_interface<impl_t>);
}

namespace {
template <typename T, auto N>
using sub_config_t = interrupt::sub_irq<enable_field_t<N>, status_field_t<N>,
                                        interrupt::policies<>, T>;

template <typename T>
using flow_config_t =
    interrupt::shared_sub_irq<enable_field_t<0>, status_field_t<0>,
                              interrupt::policies<>, sub_config_t<T, 1>>;
} // namespace

TEST_CASE("impl runs a flow", "[shared_sub_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 1>, test_nexus>;
    using impl_t = interrupt::shared_sub_irq_impl<flow_config_t<std::true_type>,
                                                  sub_impl_t>;
    static_assert(impl_t::active);

    enable_field_t<0>::value = true;
    enable_field_t<1>::value = true;
    status_field_t<0>::value = true;
    status_field_t<1>::value = true;
    flow_run<std::true_type> = false;
    impl_t::run();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl is inactive when all subs are inactive",
          "[shared_sub_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 1>, test_nexus>;
    using impl_t = interrupt::shared_sub_irq_impl<flow_config_t<std::true_type>,
                                                  sub_impl_t>;
    static_assert(not impl_t::active);
}

TEST_CASE("impl is active when any sub is active", "[shared_sub_irq_impl]") {
    using active_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 17>, test_nexus>;
    using inactive_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 42>, test_nexus>;
    using impl_t =
        interrupt::shared_sub_irq_impl<flow_config_t<std::true_type>,
                                       active_sub_impl_t, inactive_sub_impl_t>;
    static_assert(impl_t::active);
}

TEST_CASE("impl is inactive when there are no subs", "[shared_sub_irq_impl]") {
    using impl_t =
        interrupt::shared_sub_irq_impl<flow_config_t<std::true_type>>;
    static_assert(not impl_t::active);
}

TEST_CASE(
    "impl enable fields are active sub enable fields plus its own enable field",
    "[shared_sub_irq_impl]") {
    using active_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, 17>, test_nexus>;
    using inactive_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, 42>, test_nexus>;
    using impl_t =
        interrupt::shared_sub_irq_impl<flow_config_t<std::true_type>,
                                       active_sub_impl_t, inactive_sub_impl_t>;

    auto expected = stdx::tuple{enable_field_t<0>{}, enable_field_t<17>{}};
    auto actual = impl_t::get_interrupt_enables();

    CHECK(std::size(expected) == std::size(actual));
    CHECK(expected == actual);
}
