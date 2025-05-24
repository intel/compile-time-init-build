#include <cib/builder_meta.hpp>

#include <stdx/compiler.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
struct TestBuilder;
struct TestInterface {};

struct test_builder_meta {
    using builder_t = TestBuilder;
    using interface_t = TestInterface;
    CONSTEVAL static auto uninitialized() -> interface_t;
};
} // namespace

TEST_CASE("builder_meta concept") {
    STATIC_REQUIRE(cib::builder_meta<test_builder_meta>);
}

TEST_CASE(
    "builder_meta builder and interface type traits return correct values") {
    STATIC_REQUIRE(
        std::is_same_v<TestBuilder, cib::builder_t<test_builder_meta>>);
    STATIC_REQUIRE(
        std::is_same_v<TestInterface, cib::interface_t<test_builder_meta>>);
}
