#include <cib/builder_meta.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
struct TestBuilderTag;
struct TestInterfaceTag;

struct test_builder_meta
    : public cib::builder_meta<TestBuilderTag, TestInterfaceTag> {};
} // namespace

TEST_CASE(
    "builder_meta builder and interface type traits return correct values") {
    REQUIRE(std::is_same_v<TestBuilderTag, cib::builder_t<test_builder_meta>>);
    REQUIRE(
        std::is_same_v<TestInterfaceTag, cib::interface_t<test_builder_meta>>);
}
