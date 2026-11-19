#include "common.hpp"
#include "dynamic_config.hpp"

#include <interrupt/manager.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>

auto enable_33_1() -> std::int64_t {
    using namespace groov::literals;
    using M = interrupt::manager<config, test_hal<G>, test_nexus>;

    // already enabled: re-enabling should not cause a write
    calls.clear();
    using D = typename M::dynamic_t;
    D::enable<"shared_33">();
    D::enable<"sub_33_1">();
    D::enable<flow_33_1>();
    auto v = groov::test::get_value<G>("enable"_r);
    REQUIRE(v);
    CHECK(*v == EN_33_1::mask<std::uint32_t>);
    return std::count(calls.cbegin(), calls.cend(), call::write);
}
