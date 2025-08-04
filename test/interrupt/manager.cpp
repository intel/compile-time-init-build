#include "common.hpp"

#include <interrupt/config.hpp>
#include <interrupt/manager.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

struct flow_1 : std::true_type {};
struct flow_2 : std::true_type {};

namespace {
using config_a =
    interrupt::root<interrupt::irq<17_irq, 42, interrupt::policies<>, flow_1>>;
using config_b = interrupt::root<
    interrupt::irq<17_irq, 42, interrupt::policies<>, flow_1, flow_2>>;
} // namespace

TEST_CASE("init enables interrupts", "[manager]") {
    auto m = interrupt::manager<config_a, test_nexus>{};
    inited = false;
    enabled<17_irq> = false;
    priority<17_irq> = 0;

    m.init();

    CHECK(inited);
    CHECK(enabled<17_irq>);
    CHECK(priority<17_irq> == 42);
}

TEST_CASE("manager can dump config", "[manager]") {
    using namespace stdx::literals;
    constexpr auto s1 = interrupt::manager<config_a, test_nexus>::config();
    STATIC_REQUIRE(
        s1 ==
        "interrupt::root<interrupt::irq<17_irq, 42, interrupt::policies<>, flow_1>>"_cts);
    constexpr auto s2 = interrupt::manager<config_b, test_nexus>::config();
    STATIC_REQUIRE(
        s2 ==
        "interrupt::root<interrupt::irq<17_irq, 42, interrupt::policies<>, flow_1, flow_2>>"_cts);
}

TEST_CASE("run single flow", "[manager]") {
    auto m = interrupt::manager<config_a, test_nexus>{};
    flow_run<flow_1> = false;

    m.run<17_irq>();

    CHECK(flow_run<flow_1>);
}

TEST_CASE("run multiple flows", "[manager]") {
    auto m = interrupt::manager<config_b, test_nexus>{};
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
    auto m = interrupt::manager<config_a, test_nexus, alt_nexus>{};
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

template <typename... Ts> struct root : interrupt::root<Ts...> {
    template <typename> struct dynamic_controller_t {
        template <bool Enable, typename... Field>
        static void enable_by_field() {
            ((Field::value = Enable), ...);
        }
    };
};

using config_shared =
    root<interrupt::shared_irq<
             33_irq, 34, interrupt::policies<>,
             interrupt::sub_irq<enable_field_t<33'1>, status_field_t<33'1>,
                                interrupt::policies<>, flow_33_1>,
             interrupt::sub_irq<enable_field_t<33'2>, status_field_t<33'2>,
                                interrupt::policies<>, flow_33_2>>,
         interrupt::irq<38_irq, 39, interrupt::policies<>, flow_38>>;
} // namespace

TEST_CASE("init enables mcu interrupts", "[manager]") {
    auto m = interrupt::manager<config_shared, test_nexus>{};
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
    auto m = interrupt::manager<config_shared, test_nexus>{};
    enable_field_t<33'1>::value = false;
    enable_field_t<33'2>::value = false;

    m.init();

    CHECK(enable_field_t<33'1>::value);
    CHECK(enable_field_t<33'2>::value);
}

TEST_CASE("run flows if sub_irq is enabled", "[manager]") {
    auto m = interrupt::manager<config_shared, test_nexus>{};
    enable_field_t<33'1>::value = true;
    status_field_t<33'1>::value = true;
    enable_field_t<33'2>::value = false;
    flow_run<flow_33_1> = false;
    flow_run<flow_33_2> = false;

    m.run<33_irq>();

    CHECK(not status_field_t<33'1>::value);
    CHECK(flow_run<flow_33_1>);
    CHECK(not flow_run<flow_33_2>);
}

TEST_CASE("init enables mcu interrupt if any flow is active", "[manager]") {
    using config_t = root<interrupt::shared_irq<
        33_irq, 34, interrupt::policies<>,
        interrupt::sub_irq<enable_field_t<33'0>, status_field_t<33'0>,
                           interrupt::policies<>, std::true_type>,
        interrupt::sub_irq<enable_field_t<33'1>, status_field_t<33'1>,
                           interrupt::policies<>, std::false_type>>>;

    enable_field_t<33'0>::value = false;
    enable_field_t<33'1>::value = false;
    auto m = interrupt::manager<config_t, test_nexus>{};
    enabled<33_irq> = false;
    priority<33_irq> = 0;

    m.init();

    CHECK(enabled<33_irq>);
    CHECK(priority<33_irq> == 34);
}

TEST_CASE("init does not enable mcu interrupt if all flows are inactive",
          "[manager]") {
    using config_t = root<interrupt::shared_irq<
        33_irq, 34, interrupt::policies<>,
        interrupt::sub_irq<enable_field_t<33'0>, status_field_t<33'0>,
                           interrupt::policies<>, std::false_type>>>;

    enable_field_t<33'0>::value = false;
    auto m = interrupt::manager<config_t, test_nexus>{};
    enabled<33_irq> = false;
    priority<33_irq> = 0;

    m.init();

    CHECK(not enabled<33_irq>);
    CHECK(priority<33_irq> == 34);
}

namespace {
struct flow_33_2_1 : std::true_type {};
struct flow_33_2_2 : std::true_type {};

using config_shared_sub = root<interrupt::shared_irq<
    33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<enable_field_t<33'1>, status_field_t<33'1>,
                       interrupt::policies<>, flow_33_1>,
    interrupt::shared_sub_irq<
        enable_field_t<33'2>, status_field_t<33'2>, interrupt::policies<>,
        interrupt::sub_irq<enable_field_t<33'2'1>, status_field_t<33'2'1>,
                           interrupt::policies<>, flow_33_2_1>,
        interrupt::sub_irq<enable_field_t<33'2'2>, status_field_t<33'2'2>,
                           interrupt::policies<>, flow_33_2_2>>>>;
} // namespace

TEST_CASE("run flows for shared sub irqs if enabled", "[manager]") {
    auto m = interrupt::manager<config_shared_sub, test_nexus>{};
    enable_field_t<33'1>::value = false;

    enable_field_t<33'2>::value = true;
    status_field_t<33'2>::value = true;

    enable_field_t<33'2'1>::value = true;
    status_field_t<33'2'1>::value = true;
    enable_field_t<33'1'2>::value = false;

    flow_run<flow_33_1> = false;
    flow_run<flow_33_2_1> = false;
    flow_run<flow_33_2_2> = false;

    m.run<33_irq>();

    CHECK(not status_field_t<33'2'1>::value);
    CHECK(not flow_run<flow_33_1>);
    CHECK(flow_run<flow_33_2_1>);
    CHECK(not flow_run<flow_33_2_2>);
}

namespace {
using config_shared_no_enable = root<interrupt::shared_irq<
    33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<interrupt::enable_t<>, status_field_t<33'1>,
                       interrupt::policies<>, flow_33_1>>>;
using config_shared_no_status = root<interrupt::shared_irq<
    33_irq, 34, interrupt::policies<>,
    interrupt::sub_irq<enable_field_t<33'1>, interrupt::status_t<>,
                       interrupt::policies<>, flow_33_1>>>;
} // namespace

TEST_CASE("run flows with no enable field", "[manager]") {
    auto m = interrupt::manager<config_shared_no_enable, test_nexus>{};
    status_field_t<33'1>::value = true;
    flow_run<flow_33_1> = false;

    m.run<33_irq>();

    CHECK(not status_field_t<33'2'1>::value);
    CHECK(flow_run<flow_33_1>);
}

TEST_CASE("run flows with no status field", "[manager]") {
    auto m = interrupt::manager<config_shared_no_status, test_nexus>{};
    enable_field_t<33'1>::value = true;
    flow_run<flow_33_1> = false;

    m.run<33_irq>();

    CHECK(flow_run<flow_33_1>);
}

namespace {
using config_shared_id = root<interrupt::shared_irq<
    33_irq, 34, interrupt::policies<>,
    interrupt::id_irq<enable_field_t<33'0>, interrupt::policies<>>>>;
} // namespace

TEST_CASE("init enables id interrupts", "[manager]") {
    auto m = interrupt::manager<config_shared_id, test_nexus>{};

    enable_field_t<33'0>::value = false;
    m.init();
    CHECK(enable_field_t<33'0>::value);
}
