#include "common.hpp"

#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/manager.hpp>
#include <interrupt/policies.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
struct with_enable_field {
    constexpr static auto enable_field = 0;
};
struct without_enable_field {};
} // namespace

TEST_CASE("detect enable_field", "[dynamic controller]") {
    static_assert(interrupt::has_enable_field<with_enable_field>);
    static_assert(not interrupt::has_enable_field<without_enable_field>);
}

namespace {
using namespace interrupt;

std::uint32_t register_value{};
constexpr auto write(auto v) {
    return [=] { register_value = v.value; };
}

using test_flow_1_t = irq_flow<"1">;
using test_flow_2_t = irq_flow<"2">;

struct reg_t : mock_register_t<0, reg_t, ""> {};
using en_field_1_t = mock_field_t<1, reg_t, "", 0, 0>;
using sts_field_t = mock_field_t<2, reg_t, "", 1, 1>;
using en_field_2_t = mock_field_t<3, reg_t, "", 2, 2>;

struct test_resource_1 {};
struct test_resource_2 {};

using config_t = root<shared_irq<
    0_irq, 0, policies<>,
    sub_irq<en_field_1_t, sts_field_t, test_flow_1_t,
            policies<required_resources<test_resource_1>>>,
    sub_irq<en_field_2_t, sts_field_t, test_flow_2_t,
            policies<required_resources<test_resource_1, test_resource_2>>>>>;

using dynamic_t = dynamic_controller<config_t>;

auto reset_dynamic_state() {
    register_value = 0;
    dynamic_t::disable<test_flow_1_t, test_flow_2_t>();
    dynamic_t::turn_on_resource<test_resource_1>();
    dynamic_t::turn_on_resource<test_resource_2>();
}
} // namespace

TEST_CASE("enable one irq", "[dynamic controller]") {
    reset_dynamic_state();
    dynamic_t::enable<test_flow_1_t>();
    CHECK(register_value == 0b1);
}

TEST_CASE("enable multiple irqs", "[dynamic controller]") {
    reset_dynamic_state();
    dynamic_t::enable<test_flow_1_t, test_flow_2_t>();
    CHECK(register_value == 0b101);
}

TEST_CASE("disabling resource disables irq that requires it",
          "[dynamic controller]") {
    reset_dynamic_state();

    dynamic_t::enable<test_flow_2_t>();
    CHECK(register_value == 0b100);

    dynamic_t::turn_off_resource<test_resource_2>();
    CHECK(register_value == 0);
    dynamic_t::turn_on_resource<test_resource_2>();
    CHECK(register_value == 0b100);
}

TEST_CASE("disabling resource disables only irqs that require it",
          "[dynamic controller]") {
    reset_dynamic_state();

    dynamic_t::enable<test_flow_1_t, test_flow_2_t>();
    CHECK(register_value == 0b101);

    dynamic_t::turn_off_resource<test_resource_2>();
    CHECK(register_value == 0b1);
    dynamic_t::turn_on_resource<test_resource_2>();
    CHECK(register_value == 0b101);
}

TEST_CASE("disable resource that multiple irqs require",
          "[dynamic controller]") {
    reset_dynamic_state();

    dynamic_t::enable<test_flow_1_t, test_flow_2_t>();
    CHECK(register_value == 0b101);

    dynamic_t::turn_off_resource<test_resource_1>();
    CHECK(register_value == 0);
    dynamic_t::turn_on_resource<test_resource_1>();
    CHECK(register_value == 0b101);
}
