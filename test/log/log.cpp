#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
bool panicked{};

struct injected_handler {
    template <stdx::ct_string Why, typename... Ts>
    static auto panic(Ts &&...) noexcept -> void {
        static_assert(std::string_view{Why} == "Hello");
        panicked = true;
    }
};
} // namespace

template <> inline auto stdx::panic_handler<> = injected_handler{};

TEST_CASE("FATAL calls compile-time panic", "[log]") {
    panicked = false;
    CIB_FATAL("Hello");
    CHECK(panicked);
}
