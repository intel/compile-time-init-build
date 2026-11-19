#include "common.hpp"
#include "dynamic_config.hpp"

#include <interrupt/manager.hpp>

#include <groov/test.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

extern auto enable_33_1() -> std::int64_t;

TEST_CASE("run flow from another TU", "[dynamic_multi_tu]") {
    using namespace groov::literals;
    groov::test::reset_store<G>();

    // init sets internal state and shadow registers
    using M = interrupt::manager<config, test_hal<G>, test_nexus>;
    M::init<only_flow_33_1_policy_t>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN_33_1::mask<std::uint32_t>);

    // already enabled: re-enabling should not cause a write
    calls.clear();
    using D = typename M::dynamic_t;
    D::enable<"shared_33">();
    D::enable<"sub_33_1">();
    D::enable<flow_33_1>();
    v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN_33_1::mask<std::uint32_t>);
    CHECK(std::count(calls.cbegin(), calls.cend(), call::write) == 0);

    // re-enabling from another TU should not cause a write, assuming the shadow
    // state is not per-TU
    CHECK(enable_33_1() == 0);
}
