#include "common.hpp"

#include <flow/flow.hpp>
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

using EN0 = groov::field<"0", std::uint8_t, 0, 0>;
using EN1 = groov::field<"1", std::uint8_t, 1, 1>;
using EN2 = groov::field<"2", std::uint8_t, 2, 2>;

using ST1 = groov::field<"1", std::uint8_t, 1, 1>;
using ST2 = groov::field<"2", std::uint8_t, 2, 2>;

using R_ENABLE =
    groov::reg<"enable", std::uint32_t, 1234, groov::w::replace, EN0, EN1, EN2>;
using R_STATUS =
    groov::reg<"status", std::uint32_t, 5678, groov::w::replace, ST1, ST2>;

using G = groov::group<"test", groov::test::bus<"test">, R_ENABLE, R_STATUS>;

using interrupt::operator""_irq;

struct test_flow_1_t : public flow::service<"1"> {};
struct test_flow_2_t : public flow::service<"2"> {};

struct test_resource_0;
struct test_resource_1;
struct test_resource_2;

using config_t = interrupt::root<interrupt::shared_irq<
    "shared", 0_irq, 0, interrupt::policies<>,
    interrupt::id_irq<
        "id", en_field_t<"0">,
        interrupt::policies<interrupt::required_resources<test_resource_0>>>,
    interrupt::sub_irq<
        "sub1", en_field_t<"1">, st_field_t<"1">,
        interrupt::policies<interrupt::required_resources<test_resource_1>>,
        test_flow_1_t>,
    interrupt::sub_irq<"sub2", en_field_t<"2">, st_field_t<"2">,
                       interrupt::policies<interrupt::required_resources<
                           test_resource_1, test_resource_2>>,
                       test_flow_2_t>>>;

using dynamic_t = interrupt::dynamic_controller<config_t, test_hal<G>>;

namespace {
auto reset_dynamic_state() -> void {
    dynamic_t::init<false>();
    groov::test::reset_store<G>();
}
} // namespace

TEST_CASE("control IRQ by name", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    dynamic_t::disable<"id">();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN1::mask<std::uint32_t> | EN2::mask<std::uint32_t>));

    dynamic_t::enable<"id">();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == (EN0::mask<std::uint32_t> | EN1::mask<std::uint32_t> |
                 EN2::mask<std::uint32_t>));
}

TEST_CASE("name control is thread-safe", "[dynamic controller]") {
    using namespace groov::literals;
    reset_dynamic_state();
    dynamic_t::init();

    auto t1 = std::thread([&] { dynamic_t::disable<"id">(); });
    auto t2 = std::thread([&] { dynamic_t::disable<"sub1">(); });
    t1.join();
    t2.join();

    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN2::mask<std::uint32_t>);

    auto t3 = std::thread([&] { dynamic_t::enable<"id">(); });
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

    dynamic_t::enable<"id">();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(not v);

    dynamic_t::enable<test_resource_0>();
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
