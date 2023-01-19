#include <interrupt/dynamic_controller.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
struct with_enable_field {
    constexpr static auto enable_field = 0;
};
struct without_enable_field {};
} // namespace

TEST_CASE("detect enable_field", "[dynamic_controller]") {
    static_assert(interrupt::has_enable_field<with_enable_field>);
    static_assert(not interrupt::has_enable_field<without_enable_field>);
}
