#include "common.hpp"

#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/manager.hpp>

#include <groov/groov.hpp>
#include <groov/test.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <thread>
#include <type_traits>

struct flow_1 : std::true_type {};
struct flow_2 : std::true_type {};

namespace {
using config_a = interrupt::root<
    interrupt::irq<"a", 17_irq, 42, interrupt::policies<>, flow_1>>;
using config_b = interrupt::root<
    interrupt::irq<"b", 17_irq, 42, interrupt::policies<>, flow_1, flow_2>>;

using EN_33_0 = groov::field<"33_0", std::uint8_t, 0, 0>;
using EN_33_1 = groov::field<"33_1", std::uint8_t, 1, 1>;
using EN_33_2 = groov::field<"33_2", std::uint8_t, 2, 2>;
using EN_33_2_1 = groov::field<"33_2_1", std::uint8_t, 3, 3>;
using EN_33_2_2 = groov::field<"33_2_2", std::uint8_t, 4, 4>;

using ST_33_0 = groov::field<"33_0", std::uint8_t, 0, 0>;
using ST_33_1 = groov::field<"33_1", std::uint8_t, 1, 1>;
using ST_33_2 = groov::field<"33_2", std::uint8_t, 2, 2>;
using ST_33_2_1 = groov::field<"33_2_1", std::uint8_t, 3, 3>;
using ST_33_2_2 = groov::field<"33_2_2", std::uint8_t, 4, 4>;

using R_ENABLE = groov::reg<"enable", std::uint32_t, 1234, groov::w::replace,
                            EN_33_0, EN_33_1, EN_33_2, EN_33_2_1, EN_33_2_2>;
using R_STATUS = groov::reg<"status", std::uint32_t, 5678, groov::w::replace,
                            ST_33_0, ST_33_1, ST_33_2, ST_33_2_1, ST_33_2_2>;

using G = groov::group<"test", groov::test::bus<"test">, R_ENABLE, R_STATUS>;
} // namespace

TEST_CASE("manager can dump config", "[manager]") {
    using namespace stdx::literals;
    constexpr auto s1 =
        interrupt::manager<config_a, test_hal<G>, test_nexus>::config();
    STATIC_CHECK(
        s1 ==
        "interrupt::root<interrupt::irq<\"a\", 17_irq, 42, interrupt::policies<>, flow_1>>"_cts);
    constexpr auto s2 =
        interrupt::manager<config_b, test_hal<G>, test_nexus>::config();
    STATIC_CHECK(
        s2 ==
        "interrupt::root<interrupt::irq<\"b\", 17_irq, 42, interrupt::policies<>, flow_1, flow_2>>"_cts);
}

TEST_CASE("run single flow", "[manager]") {
    auto m = interrupt::manager<config_a, test_hal<G>, test_nexus>{};
    flow_run<flow_1> = false;

    m.run<17_irq>();

    CHECK(flow_run<flow_1>);
}

TEST_CASE("run multiple flows", "[manager]") {
    auto m = interrupt::manager<config_b, test_hal<G>, test_nexus>{};
    flow_run<flow_1> = false;
    flow_run<flow_2> = false;

    m.run<17_irq>();

    CHECK(flow_run<flow_1>);
    CHECK(flow_run<flow_2>);
}

namespace {
template <typename Flow> struct alt_flow : Flow {};

struct alt_nexus {
    template <typename T> constexpr static auto service = flow_t<alt_flow<T>>{};
};
} // namespace

TEST_CASE("run flow across multiple nexi", "[manager]") {
    auto m = interrupt::manager<config_a, test_hal<G>, test_nexus, alt_nexus>{};
    flow_run<flow_1> = false;
    flow_run<alt_flow<flow_1>> = false;

    m.run<17_irq>();

    CHECK(flow_run<flow_1>);
    CHECK(flow_run<alt_flow<flow_1>>);
}

namespace {
struct flow_33_1 : std::true_type {};
struct flow_33_2 : std::true_type {};
struct flow_38 : std::true_type {};

using config_shared = interrupt::root<
    interrupt::shared_irq<
        "shared_33", 33_irq, 34, interrupt::policies<>,
        interrupt::sub_irq<"sub_33_1", en_field_t<"33_1">, st_field_t<"33_1">,
                           interrupt::policies<>, flow_33_1>,
        interrupt::sub_irq<"sub_33_2", en_field_t<"33_2">, st_field_t<"33_2">,
                           interrupt::policies<>, flow_33_2>>,
    interrupt::irq<"irq_38", 38_irq, 39, interrupt::policies<>, flow_38>>;
} // namespace

TEST_CASE("init enables mcu interrupts", "[manager]") {
    auto m = interrupt::manager<config_shared, test_hal<G>, test_nexus>{};
    inited = false;
    enabled<33_irq> = false;
    priority<33_irq> = 0;
    enabled<38_irq> = false;
    priority<38_irq> = 0;

    m.init();
    CHECK(38_irq == m.max_irq());

    CHECK(inited);
    CHECK(enabled<33_irq>);
    CHECK(priority<33_irq> == 34);
    CHECK(enabled<38_irq>);
    CHECK(priority<38_irq> == 39);
}

TEST_CASE("init enables dynamic interrupts", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    auto m = interrupt::manager<config_shared, test_hal<G>, test_nexus>{};
    m.init();

    auto expected = EN_33_1::mask<std::uint32_t> | EN_33_2::mask<std::uint32_t>;
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == expected);
}

TEST_CASE("run flows if sub_irq is enabled", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    auto m = interrupt::manager<config_shared, test_hal<G>, test_nexus>{};
    m.init();

    groov::test::set_value<G>("status"_r, EN_33_1::mask<std::uint32_t>);
    m.run<33_irq>();

    auto v = groov::test::get_value<G>("status"_r);
    REQUIRE(v);
    CHECK(*v == 0);

    CHECK(flow_run<flow_33_1>);
    CHECK(not flow_run<flow_33_2>);
}

TEST_CASE("sub_irq run is thread-safe", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    using M = interrupt::manager<config_shared, test_hal<G>, test_nexus>;
    using D = typename M::dynamic_t;

    auto m = M{};
    m.init();

    groov::test::set_value<G>("status"_r, EN_33_1::mask<std::uint32_t>);

    auto t1 = std::thread([&] { D::disable<"sub_33_1">(); });
    auto t2 = std::thread([&] { m.run<33_irq>(); });
    t1.join();
    t2.join();
}

TEST_CASE("init enables mcu interrupt if any flow is active", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    using config_t = interrupt::root<interrupt::shared_irq<
        "shared_33", 33_irq, 34, interrupt::policies<>,
        interrupt::sub_irq<"sub_33_0", en_field_t<"33_0">, st_field_t<"33_0">,
                           interrupt::policies<>, std::true_type>,
        interrupt::sub_irq<"sub_33_1", en_field_t<"33_1">, st_field_t<"33_1">,
                           interrupt::policies<>, std::false_type>>>;

    auto m = interrupt::manager<config_t, test_hal<G>, test_nexus>{};
    enabled<33_irq> = false;
    priority<33_irq> = 0;

    m.init();

    CHECK(enabled<33_irq>);
    CHECK(priority<33_irq> == 34);
}

TEST_CASE("init does not enable mcu interrupt if all flows are inactive",
          "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    using config_t = interrupt::root<interrupt::shared_irq<
        "shared_33", 33_irq, 34, interrupt::policies<>,
        interrupt::sub_irq<"sub_33_0", en_field_t<"33_0">, st_field_t<"33_0">,
                           interrupt::policies<>, std::false_type>>>;

    auto m = interrupt::manager<config_t, test_hal<G>, test_nexus>{};
    enabled<33_irq> = false;
    priority<33_irq> = 0;

    m.init();

    CHECK(not enabled<33_irq>);
    CHECK(priority<33_irq> == 34);
}

namespace {
struct flow_33_2_1 : std::true_type {};
struct flow_33_2_2 : std::true_type {};

using config_shared_sub = interrupt::root<interrupt::shared_irq<
    "shared_33", 33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<"sub_33_1", en_field_t<"33_1">, st_field_t<"33_1">,
                       interrupt::policies<>, flow_33_1>,
    interrupt::shared_sub_irq<
        "shared_sub_33_2", en_field_t<"33_2">, st_field_t<"33_2">,
        interrupt::policies<>,
        interrupt::sub_irq<"sub_33_2_1", en_field_t<"33_2_1">,
                           st_field_t<"33_2_1">, interrupt::policies<>,
                           flow_33_2_1>,
        interrupt::sub_irq<"sub_33_2_2", en_field_t<"33_2_2">,
                           st_field_t<"33_2_2">, interrupt::policies<>,
                           flow_33_2_2>>>>;
} // namespace

TEST_CASE("run flows for shared sub irqs if enabled", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();
    auto m = interrupt::manager<config_shared_sub, test_hal<G>, test_nexus>{};
    m.init();

    auto fields = EN_33_2::mask<std::uint32_t> | EN_33_2_1::mask<std::uint32_t>;
    groov::test::set_value<G>("enable"_r, fields);
    groov::test::set_value<G>("status"_r, fields);

    flow_run<flow_33_1> = false;
    flow_run<flow_33_2_1> = false;
    flow_run<flow_33_2_2> = false;

    m.run<33_irq>();

    auto v = groov::test::get_value<G>("status"_r);
    REQUIRE(v);
    CHECK(*v == 0);

    CHECK(not flow_run<flow_33_1>);
    CHECK(flow_run<flow_33_2_1>);
    CHECK(not flow_run<flow_33_2_2>);
}

TEST_CASE("shared_sub_irq run is thread-safe", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    using M = interrupt::manager<config_shared_sub, test_hal<G>, test_nexus>;
    using D = typename M::dynamic_t;

    auto m = M{};
    m.init();

    auto fields = EN_33_2::mask<std::uint32_t> | EN_33_2_1::mask<std::uint32_t>;
    groov::test::set_value<G>("enable"_r, fields);
    groov::test::set_value<G>("status"_r, fields);

    auto t1 = std::thread([&] { D::disable<"sub_33_2_1">(); });
    auto t2 = std::thread([&] { m.run<33_irq>(); });
    t1.join();
    t2.join();
}

namespace {
using config_shared_no_enable = interrupt::root<interrupt::shared_irq<
    "shared_33", 33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<"sub_33_1", interrupt::no_field_t, st_field_t<"33_1">,
                       interrupt::policies<>, flow_33_1>>>;
using config_shared_no_status = interrupt::root<interrupt::shared_irq<
    "shared_33", 33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<"sub_33_1", en_field_t<"33_1">, interrupt::no_field_t,
                       interrupt::policies<>, flow_33_1>>>;
} // namespace

TEST_CASE("run flows with no enable field", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();
    auto m =
        interrupt::manager<config_shared_no_enable, test_hal<G>, test_nexus>{};
    m.init();

    groov::test::set_value<G>("status"_r, EN_33_1::mask<std::uint32_t>);
    flow_run<flow_33_1> = false;

    m.run<33_irq>();

    auto v = groov::test::get_value<G>("status"_r);
    REQUIRE(v);
    CHECK(*v == 0);
    CHECK(flow_run<flow_33_1>);
}

TEST_CASE("run flows with no status field", "[manager]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();
    auto m =
        interrupt::manager<config_shared_no_status, test_hal<G>, test_nexus>{};
    m.init();

    flow_run<flow_33_1> = false;

    m.run<33_irq>();

    CHECK(flow_run<flow_33_1>);
}
