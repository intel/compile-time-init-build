#include "common.hpp"

#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/manager.hpp>
#include <interrupt/policies.hpp>

#include <groov/groov.hpp>
#include <groov/test.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <thread>

namespace {
struct with_enable_field {
    using enable_field_t = int;
};
struct without_enable_field {};
struct with_no_enable_field {
    using enable_field_t = interrupt::no_field_t;
};
} // namespace

TEST_CASE("detect enable_field", "[dynamic controller]") {
    STATIC_CHECK(interrupt::detail::has_enable_field<with_enable_field>::value);
    STATIC_CHECK(
        not interrupt::detail::has_enable_field<without_enable_field>::value);
    STATIC_CHECK(
        not interrupt::detail::has_enable_field<with_no_enable_field>::value);
}

namespace {
using EN0 = groov::field<"0", std::uint8_t, 0, 0>;
using EN1 = groov::field<"1", std::uint8_t, 1, 1>;
using EN2 = groov::field<"2", std::uint8_t, 2, 2>;

using ST1 = groov::field<"1", std::uint8_t, 1, 1>;
using ST2 = groov::field<"2", std::uint8_t, 2, 2>;

using R_ENABLE =
    groov::reg<"enable", std::uint32_t, 1234, groov::w::replace, EN0, EN1, EN2>;
using R_STATUS =
    groov::reg<"status", std::uint32_t, 5678, groov::w::replace, ST1, ST2>;

using EN_TOP = groov::field<"top", std::uint8_t, 7, 7>;
using R_TOP_ENABLE =
    groov::reg<"enable_top", std::uint32_t, 5678, groov::w::replace, EN_TOP>;

using G = groov::group<"test", groov::test::bus<"test">, R_ENABLE, R_STATUS,
                       R_TOP_ENABLE>;

using interrupt::operator""_irq;

struct test_flow_0_t : std::true_type {};
struct test_flow_1_t : std::true_type {};
struct test_flow_2_t : std::true_type {};

struct test_resource_0;
struct test_resource_1;
struct test_resource_2;

using config_t = interrupt::root<interrupt::shared_irq<
    "shared", 0_irq, 0, stdx::cts_t<"enable_top.top"_cts>,
    interrupt::policies<>,
    interrupt::sub_irq<
        "sub0", en_field_t<"0">, st_field_t<"0">,
        interrupt::policies<interrupt::required_resources<test_resource_0>>,
        test_flow_0_t>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        test_flow_1_t>,
    interrupt::sub_irq<"sub2", en_field_t<"2">, st_field_t<"2">,
                       interrupt::policies<interrupt::required_resources<
                           test_resource_1, test_resource_2>>,
                       test_flow_2_t>>>;

using dynamic_t = interrupt::dynamic_controller<config_t, test_hal<G>>;

auto reset_dynamic_state() -> void {
    dynamic_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("dynamic init enables all fields", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));

    v = groov::test::get_value<G>("enable_top"_r);
    REQUIRE(v);
    CHECK(*v == EN_TOP::mask<std::uint32_t>);
}

namespace {
using noflows_config_t = interrupt::root<interrupt::shared_irq<
    "shared", 0_irq, 0, stdx::cts_t<"enable_top.top"_cts>,
    interrupt::policies<>,
    interrupt::sub_irq<
        "sub0", en_field_t<"0">, st_field_t<"0">,
        interrupt::policies<interrupt::required_resources<test_resource_0>>>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>>>>;

using noflows_dynamic_t =
    interrupt::dynamic_controller<noflows_config_t, test_hal<G>>;

auto reset_noflows_dynamic_state() -> void {
    noflows_dynamic_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("dynamic init does not enable with no flows",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_noflows_dynamic_state();
    noflows_dynamic_t::init();

    auto v = groov::test::get_value<G>("enable"_r);
    CHECK(not v);

    v = groov::test::get_value<G>("enable_top"_r);
    CHECK(not v);
}

namespace {
struct inactive_test_flow_1_t : std::false_type {};

using inactiveflow_config_t = interrupt::root<interrupt::shared_irq<
    "shared", 0_irq, 0, stdx::cts_t<"enable_top.top"_cts>,
    interrupt::policies<>,
    interrupt::sub_irq<
        "sub0", en_field_t<"0">, st_field_t<"0">,
        interrupt::policies<interrupt::required_resources<test_resource_0>>,
        test_flow_0_t>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        inactive_test_flow_1_t>>>;

using inactiveflow_dynamic_t =
    interrupt::dynamic_controller<inactiveflow_config_t, test_hal<G>>;

auto reset_inactiveflow_dynamic_state() -> void {
    inactiveflow_dynamic_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("dynamic init does not enable with inactive flow",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_inactiveflow_dynamic_state();
    using active_flows_t = boost::mp11::mp_list<test_flow_0_t>;
    inactiveflow_dynamic_t::init<true, active_flows_t>();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);

    v = groov::test::get_value<G>("enable_top"_r);
    REQUIRE(v);
    CHECK(*v == EN_TOP::mask<std::uint32_t>);
}

TEST_CASE("dynamic init refresh all enables", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    groov::test::set_value<G>("enable"_r, 0);
    groov::test::set_value<G>("enable_top"_r, 0);
    dynamic_t::refresh_all_enables();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));

    v = groov::test::get_value<G>("enable_top"_r);
    REQUIRE(v);
    CHECK(*v == EN_TOP::mask<std::uint32_t>);
}

TEST_CASE("dynamic init refresh top level", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    groov::test::set_value<G>("enable"_r, 0);
    groov::test::set_value<G>("enable_top"_r, 0);
    dynamic_t::refresh_top_level_enables();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == 0);

    v = groov::test::get_value<G>("enable_top"_r);
    REQUIRE(v);
    CHECK(*v == EN_TOP::mask<std::uint32_t>);
}

TEST_CASE("dynamic init minimizes writes", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    calls.clear();
    dynamic_t::init();
    CHECK(std::count(calls.cbegin(), calls.cend(), call::write) == 2);
}

TEST_CASE("control IRQ by name", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    dynamic_t::disable<"sub1">();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    dynamic_t::enable<"sub1">();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("name control is thread-safe", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    auto t1 = std::thread([&] { dynamic_t::disable<"sub0">(); });
    auto t2 = std::thread([&] { dynamic_t::disable<"sub1">(); });
    t1.join();
    t2.join();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN2::mask<std::uint32_t>);

    auto t3 = std::thread([&] { dynamic_t::enable<"sub0">(); });
    auto t4 = std::thread([&] { dynamic_t::enable<"sub1">(); });
    t3.join();
    t4.join();

    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("control IRQ by flow", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    dynamic_t::disable<test_flow_1_t>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    dynamic_t::enable<test_flow_1_t>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("control IRQ by resource", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    dynamic_t::disable<test_resource_0>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN1::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    dynamic_t::enable<test_resource_0>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("dynamic enable/disable minimizes writes", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();
    calls.clear();

    dynamic_t::disable<test_resource_0, test_resource_1, test_resource_2>();
    CHECK(std::count(calls.cbegin(), calls.cend(), call::write) == 1);

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == 0);
}

TEST_CASE("type control is thread-safe", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    auto t1 = std::thread([&] { dynamic_t::disable<test_resource_0>(); });
    auto t2 = std::thread([&] { dynamic_t::disable<test_resource_2>(); });
    t1.join();
    t2.join();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN1::mask<std::uint32_t>);

    auto t3 = std::thread([&] { dynamic_t::enable<test_resource_0>(); });
    auto t4 = std::thread([&] { dynamic_t::enable<test_resource_2>(); });
    t3.join();
    t4.join();

    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("IRQ is enabled only when all its contingencies are enabled",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();

    dynamic_t::enable<"sub0">();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(not v);

    dynamic_t::enable<test_resource_0>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(not v);

    dynamic_t::enable<test_flow_0_t>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);
}

TEST_CASE("disabling resource disables any IRQs that require it",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    dynamic_t::disable<test_resource_1>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);

    dynamic_t::enable<test_resource_1>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

namespace {
using shared_sub_config_t = interrupt::root<interrupt::shared_sub_irq<
    "shared0", en_field_t<"0">, st_field_t<"0">,
    interrupt::policies<interrupt::required_resources<>>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        test_flow_1_t>,
    interrupt::sub_irq<
        "sub2", en_field_t<"2">, st_field_t<"2">,
        interrupt::policies<interrupt::required_resources<test_resource_2>>,
        test_flow_2_t>>>;

using shared_sub_dynamic_t =
    interrupt::dynamic_controller<shared_sub_config_t, test_hal<G>>;

auto reset_shared_dynamic_state() -> void {
    shared_sub_dynamic_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("disabling one sub_irq by flow does not disable parent",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_flow_1_t>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    shared_sub_dynamic_t::enable<test_flow_1_t>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("disabling all sub_irqs by flow disables parent (propagate)",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_flow_1_t>();
    shared_sub_dynamic_t::disable<test_flow_2_t>(interrupt::propagate);
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == 0);

    shared_sub_dynamic_t::enable<test_flow_1_t>(interrupt::propagate);
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t>));
}

TEST_CASE(
    "disabling all sub_irqs by flow does not disable parent (default policy)",
    "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_flow_1_t>();
    shared_sub_dynamic_t::disable<test_flow_2_t>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);

    shared_sub_dynamic_t::enable<test_flow_1_t>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t>));
}

TEST_CASE("disabling one sub_irq by resource does not disable parent",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_resource_1>(interrupt::propagate);
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    shared_sub_dynamic_t::enable<test_resource_1>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("disabling all sub_irqs by resource disables parent without its own "
          "resources (propagate)",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_resource_1>();
    shared_sub_dynamic_t::disable<test_resource_2>(interrupt::propagate);
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == 0);

    shared_sub_dynamic_t::enable<test_resource_1>(interrupt::propagate);
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t>));
}

TEST_CASE("disabling all sub_irqs by resource does not disable parent without "
          "its own resources (default policy)",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic_state();
    shared_sub_dynamic_t::init();

    shared_sub_dynamic_t::disable<test_resource_1>();
    shared_sub_dynamic_t::disable<test_resource_2>(interrupt::no_propagate);
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);

    shared_sub_dynamic_t::enable<test_resource_1>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t>));
}

namespace {
using shared_sub_config2_t = interrupt::root<interrupt::shared_sub_irq<
    "shared0", en_field_t<"0">, st_field_t<"0">,
    interrupt::policies<interrupt::required_resources<test_resource_0>>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        test_flow_1_t>,
    interrupt::sub_irq<
        "sub2", en_field_t<"2">, st_field_t<"2">,
        interrupt::policies<interrupt::required_resources<test_resource_2>>,
        test_flow_2_t>>>;

using shared_sub_dynamic2_t =
    interrupt::dynamic_controller<shared_sub_config2_t, test_hal<G>>;

auto reset_shared_dynamic2_state() -> void {
    shared_sub_dynamic2_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("disabling all sub_irqs by resource does not disable parent with its "
          "own resources",
          "[dynamic controller]") {
    using namespace groov::literals;
    reset_shared_dynamic2_state();
    shared_sub_dynamic2_t::init();

    shared_sub_dynamic2_t::disable<test_resource_1>();
    shared_sub_dynamic2_t::disable<test_resource_2>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN0::mask<std::uint32_t>);
}

TEST_CASE("enabling/disabling unknown flows can be ignored",
          "[dynamic controller]") {
    dynamic_t::disable<void>(interrupt::ignore_unknowns);
}
