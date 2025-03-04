#include "catalog_concurrency.hpp"
#include "catalog_enums.hpp"

#include <conc/concurrency.hpp>
#include <log/catalog/mipi_encoder.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;

namespace {
struct test_log_args_destination {
    auto log_by_args(std::uint32_t, auto...) -> void { ++log_calls; }
    template <std::size_t N>
    auto log_by_buf(stdx::span<std::uint8_t const, N>) const {
        ++log_calls;
    }
};

using log_env2a =
    logging::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_two_rt_args() -> void;

auto log_two_rt_args() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env2a>(
        format("D string with {} and {} placeholder"_sc, std::uint32_t{1},
               std::int64_t{2}));
}
