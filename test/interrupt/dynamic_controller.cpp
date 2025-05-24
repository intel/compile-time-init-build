#include <flow/flow.hpp>
#include <interrupt/config.hpp>
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
    STATIC_REQUIRE(interrupt::has_enable_field<with_enable_field>);
    STATIC_REQUIRE(not interrupt::has_enable_field<without_enable_field>);
}

namespace {
template <typename Field> struct field_value_t {
    std::uint32_t value;
};

template <int Id, typename Reg, std::uint32_t Msb, std::uint32_t Lsb>
struct mock_field_t {
    using RegisterType = Reg;

    constexpr static auto get_register() -> Reg { return {}; }

    constexpr static auto get_mask() -> typename Reg::DataType {
        return ((1u << (Msb + 1u)) - 1u) - ((1u << Lsb) - 1u);
    }

    constexpr auto operator()(std::uint32_t value) const {
        return field_value_t<mock_field_t>{value};
    }
};

struct mock_register_t {
    using DataType = std::uint32_t;
    constexpr static mock_field_t<-1, mock_register_t, 31, 0> raw{};
};

template <typename... Ops> constexpr auto apply(Ops... ops) {
    return (ops(), ...);
}
using interrupt::operator""_irq;

std::uint32_t register_value{};
template <typename F> constexpr auto write(field_value_t<F> v) {
    return [=] { register_value = v.value; };
}

struct test_flow_1_t : public flow::service<"1"> {};
struct test_flow_2_t : public flow::service<"2"> {};

using en_field_1_t = mock_field_t<1, mock_register_t, 0, 0>;
using sts_field_t = mock_field_t<2, mock_register_t, 1, 1>;
using en_field_2_t = mock_field_t<3, mock_register_t, 2, 2>;

struct test_resource_1;
struct test_resource_2;

using config_t = interrupt::root<interrupt::shared_irq<
    0_irq, 0, interrupt::policies<>,
    interrupt::sub_irq<
        en_field_1_t, sts_field_t,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        test_flow_1_t>,
    interrupt::sub_irq<en_field_2_t, sts_field_t,
                       interrupt::policies<interrupt::required_resources<
                           test_resource_1, test_resource_2>>,
                       test_flow_2_t>>>;

using dynamic_t = interrupt::dynamic_controller<config_t>;

auto reset_dynamic_state() -> void {
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
