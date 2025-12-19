#include <log/level.hpp>
#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>
#include <stdx/type_traits.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>

template <typename> auto catalog() -> string_id { return 0xdeadbeef; }
template <typename> auto module() -> module_id { return 0x5a; }

namespace {
enum struct custom_level { THE_ONE_LEVEL = 5 };

int log_calls{};

struct test_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        using namespace msg;
        auto const msg =
            msg::const_view<logging::mipi::defn::catalog_msg_t>{pkt};
        CHECK(msg.get("severity"_f) ==
              stdx::to_underlying(custom_level::THE_ONE_LEVEL));
        ++log_calls;
    }
};
} // namespace

TEST_CASE("mipi logger works with custom level", "[level]") {
    log_calls = 0;
    CIB_LOG_ENV(logging::get_level, custom_level::THE_ONE_LEVEL);
    auto cfg = logging::binary::config{test_destination{}};
    cfg.logger.log_msg<cib_log_env_t>(stdx::ct_format<"Hello {} {}">(17, 42));
    CHECK(log_calls == 1);
}
