#include <cib/builder_meta.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {
struct TestBuilderTag;
struct TestInterfaceTag;

struct test_builder_meta {
    using builder_t = TestBuilderTag;
    using interface_t = TestInterfaceTag;
};
} // namespace

TEST_CASE("builder_meta concept") {
    static_assert(cib::builder_meta<test_builder_meta>);
}

TEST_CASE(
    "builder_meta builder and interface type traits return correct values") {
    static_assert(
        std::is_same_v<TestBuilderTag, cib::builder_t<test_builder_meta>>);
    static_assert(
        std::is_same_v<TestInterfaceTag, cib::interface_t<test_builder_meta>>);
}
