#include "common.hpp"

#include <interrupt/concepts.hpp>

#include <groov/test.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
using EN0 = groov::field<"0", std::uint8_t, 0, 0>;
using ST0 = groov::field<"0", std::uint8_t, 0, 0>;

using R_ENABLE =
    groov::reg<"enable", std::uint32_t, 1234, groov::w::replace, EN0>;
using R_STATUS =
    groov::reg<"status", std::uint32_t, 5678, groov::w::replace, ST0>;

using G = groov::group<"test", groov::test::bus<"test">, R_ENABLE, R_STATUS>;

using no_flows_config_t =
    interrupt::sub_irq<"test", en_field_t<"0">, st_field_t<"0">,
                       interrupt::policies<>>;
} // namespace

TEST_CASE("config models concept", "[sub_irq_impl]") {
    STATIC_CHECK(interrupt::sub_irq_config<no_flows_config_t>);
}

TEST_CASE("config default status policy is clear first", "[sub_irq_impl]") {
    STATIC_CHECK(std::is_same_v<no_flows_config_t::status_policy_t,
                                interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[sub_irq_impl]") {
    using config_t =
        interrupt::sub_irq<"test", void, void,
                           interrupt::policies<interrupt::clear_status_last>>;
    STATIC_CHECK(std::is_same_v<config_t::status_policy_t,
                                interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[sub_irq_impl]") {
    using impl_t = interrupt::sub_irq_impl<no_flows_config_t, test_nexus>;
    STATIC_CHECK(interrupt::sub_irq_interface<impl_t>);
}

TEST_CASE("impl can dump config (no flows)", "[sub_irq_impl]") {
    using namespace stdx::literals;
    using config_t =
        interrupt::sub_irq<"test", void, void, interrupt::policies<>>;
    using impl_t = interrupt::sub_irq_impl<config_t, test_nexus>;
    constexpr auto s = impl_t::config();
    STATIC_CHECK(
        s ==
        "interrupt::sub_irq<\"test\", void, void, interrupt::policies<>>"_cts);
}

TEST_CASE("impl can dump config (some flows)", "[sub_irq_impl]") {
    using namespace stdx::literals;
    using config_t = interrupt::sub_irq<"test", void, void,
                                        interrupt::policies<>, std::true_type>;
    using impl_t = interrupt::sub_irq_impl<config_t, test_nexus>;
    constexpr auto s = impl_t::config();
    STATIC_CHECK(
        s ==
        "interrupt::sub_irq<\"test\", void, void, interrupt::policies<>, std::integral_constant<bool, true>>"_cts);
}

namespace {
template <typename T>
using flow_config_t =
    interrupt::sub_irq<"test", en_field_t<"0">, st_field_t<"0">,
                       interrupt::policies<>, T>;
}

TEST_CASE("impl runs a flow when enabled and status", "[sub_irq_impl]") {
    using namespace groov::literals;

    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_CHECK(impl_t::active);

    groov::test::reset_store<G>();
    groov::test::set_value<G>("enable"_r, EN0::mask<std::uint32_t>);
    groov::test::set_value<G>("status"_r, ST0::mask<std::uint32_t>);

    flow_run<std::true_type> = false;
    impl_t::run<test_hal<G>>();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl doesn't run a flow when not enabled", "[sub_irq_impl]") {
    using namespace groov::literals;

    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_CHECK(impl_t::active);

    groov::test::reset_store<G>();
    groov::test::set_value<G>("enable"_r, 0);
    groov::test::set_value<G>("status"_r, ST0::mask<std::uint32_t>);

    flow_run<std::true_type> = false;
    impl_t::run<test_hal<G>>();
    CHECK(not flow_run<std::true_type>);
}

TEST_CASE("impl doesn't run a flow when not status", "[sub_irq_impl]") {
    using namespace groov::literals;

    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::true_type>, test_nexus>;
    STATIC_CHECK(impl_t::active);

    groov::test::reset_store<G>();
    groov::test::set_value<G>("enable"_r, EN0::mask<std::uint32_t>);
    groov::test::set_value<G>("status"_r, 0);

    flow_run<std::true_type> = false;
    impl_t::run<test_hal<G>>();
    CHECK(not flow_run<std::true_type>);
}

TEST_CASE("impl is inactive when flow is not active", "[sub_irq_impl]") {
    using impl_t =
        interrupt::sub_irq_impl<flow_config_t<std::false_type>, test_nexus>;
    STATIC_CHECK(not impl_t::active);
}

TEST_CASE("impl is inactive when there are no flows", "[sub_irq_impl]") {
    using impl_t = interrupt::sub_irq_impl<no_flows_config_t, test_nexus>;
    STATIC_CHECK(not impl_t::active);
}
