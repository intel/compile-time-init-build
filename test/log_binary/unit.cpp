#include <log/level.hpp>
#include <log/unit.hpp>
#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>

template <typename> auto catalog() -> string_id { return 0xdeadbeef; }
template <typename> auto module() -> module_id { return 0x5a; }

namespace {
int log_calls{};
logging::mipi::unit_t test_unit{};

struct test_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        using namespace msg;
        auto const msg =
            msg::const_view<logging::mipi::defn::catalog_msg_t>{pkt};
        CHECK(msg.get("unit"_f) == test_unit);
        ++log_calls;
    }
};

} // namespace

TEST_CASE("mipi logger works with init-time unit", "[unit]") {
    log_calls = 0;
    test_unit = 5;
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE, logging::get_unit,
                [] { return test_unit; });
    auto cfg = logging::binary::config{test_destination{}};
    cfg.logger.log_msg<cib_log_env_t>(stdx::ct_format<"Hello {} {}">(42, 17));
    CHECK(log_calls == 1);
}

TEST_CASE("default unit is 0", "[unit]") {
    log_calls = 0;
    test_unit = 0;
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    auto cfg = logging::binary::config{test_destination{}};
    cfg.logger.log_msg<cib_log_env_t>(stdx::ct_format<"Hello {} {}">(42, 17));
    CHECK(log_calls == 1);
}
