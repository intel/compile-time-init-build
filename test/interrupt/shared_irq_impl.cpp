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
    interrupt::shared_irq<"test", 42_irq, 17, interrupt::policies<>>;
} // namespace

TEST_CASE("config models concept", "[shared_irq_impl]") {
    STATIC_CHECK(interrupt::irq_config<no_flows_config_t>);
}

TEST_CASE("config can enable/disable its irq", "[shared_irq_impl]") {
    enabled<42_irq> = false;
    no_flows_config_t::enable<true, test_hal<G>>();
    CHECK(enabled<42_irq>);
    no_flows_config_t::enable<false, test_hal<G>>();
    CHECK(not enabled<42_irq>);
}

TEST_CASE("config enables its irq with priority", "[shared_irq_impl]") {
    priority<42_irq> = 0;
    no_flows_config_t::enable<true, test_hal<G>>();
    CHECK(priority<42_irq> == 17);
}

TEST_CASE("config default status policy is clear first", "[shared_irq_impl]") {
    STATIC_CHECK(std::is_same_v<no_flows_config_t::status_policy_t,
                                interrupt::clear_status_first>);
}

TEST_CASE("config status policy can be supplied", "[shared_irq_impl]") {
    using config_t = interrupt::shared_irq<
        "test", 42_irq, 17, interrupt::policies<interrupt::clear_status_last>>;
    STATIC_CHECK(std::is_same_v<config_t::status_policy_t,
                                interrupt::clear_status_last>);
}

TEST_CASE("impl models concept", "[shared_irq_impl]") {
    using impl_t = interrupt::shared_irq_impl<no_flows_config_t>;
    STATIC_CHECK(interrupt::irq_interface<impl_t>);
}

TEST_CASE("impl can dump config (no subs)", "[shared_irq_impl]") {
    using namespace stdx::literals;
    using impl_t = interrupt::shared_irq_impl<no_flows_config_t>;
    constexpr auto s = impl_t::config();
    STATIC_CHECK(
        s ==
        "interrupt::shared_irq<\"test\", 42_irq, 17, interrupt::policies<>>"_cts);
}

TEST_CASE("impl can dump config (some subs)", "[shared_irq_impl]") {
    using namespace stdx::literals;
    using impl_t = interrupt::shared_irq<
        "test_shared", 17_irq, 42, interrupt::policies<>,
        interrupt::sub_irq<"test_sub", void, void, interrupt::policies<>,
                           std::true_type>>;

    constexpr auto s = impl_t::config();
    STATIC_CHECK(
        s ==
        "interrupt::shared_irq<\"test_shared\", 17_irq, 42, interrupt::policies<>, interrupt::sub_irq<\"test_sub\", void, void, interrupt::policies<>, std::integral_constant<bool, true>>>"_cts);
}

namespace {
template <typename T, stdx::ct_string S>
using sub_config_t =
    interrupt::sub_irq<"test_sub", en_field_t<S>, st_field_t<S>,
                       interrupt::policies<>, T>;

template <typename T>
using flow_config_t =
    interrupt::shared_irq<"test_shared", 17_irq, 42, interrupt::policies<>,
                          sub_config_t<T, "0">>;
} // namespace

TEST_CASE("impl runs a flow", "[shared_irq_impl]") {
    using namespace groov::literals;

    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, "0">, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;
    STATIC_CHECK(impl_t::active);

    groov::test::reset_store<G>();
    groov::test::set_value<G>("enable"_r, EN0::mask<std::uint32_t>);
    groov::test::set_value<G>("status"_r, ST0::mask<std::uint32_t>);

    flow_run<std::true_type> = false;
    impl_t::run<test_hal<G>>();
    CHECK(flow_run<std::true_type>);
}

TEST_CASE("impl can init its interrupt", "[shared_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, "0">, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;

    enabled<17_irq> = false;
    priority<17_irq> = 0;
    impl_t::init<test_hal<G>>();
    CHECK(enabled<17_irq>);
    CHECK(priority<17_irq> == 42);
}

TEST_CASE("impl is inactive when all subs are inactive", "[shared_irq_impl]") {
    using sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, "0">, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>, sub_impl_t>;
    STATIC_CHECK(not impl_t::active);
}

TEST_CASE("impl is active when any sub is active", "[shared_irq_impl]") {
    using active_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::true_type, "1">, test_nexus>;
    using inactive_sub_impl_t =
        interrupt::sub_irq_impl<sub_config_t<std::false_type, "2">, test_nexus>;
    using impl_t =
        interrupt::shared_irq_impl<flow_config_t<std::true_type>,
                                   active_sub_impl_t, inactive_sub_impl_t>;
    STATIC_CHECK(impl_t::active);
}

TEST_CASE("impl is inactive when there are no subs", "[shared_irq_impl]") {
    using impl_t = interrupt::shared_irq_impl<flow_config_t<std::true_type>>;
    STATIC_CHECK(not impl_t::active);
}
