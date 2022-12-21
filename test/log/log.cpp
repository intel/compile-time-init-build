#include <log/log.hpp>

#include <catch2/catch_test_macros.hpp>

static bool terminated{};
struct test_config : logging::null::config {
    static auto terminate() noexcept -> void { terminated = true; }
};

template <> inline auto logging::config<> = test_config{};

TEST_CASE("FATAL calls terminate", "[log]") {
    CIB_FATAL("Hello");
    REQUIRE(terminated);
}
