#include "catalog_concurrency.hpp"
#include "catalog_destination.hpp"

#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>

#include <conc/concurrency.hpp>

#include <cstdint>

namespace {
using log_env2a = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_two_rt_args() -> void;

auto log_two_rt_args() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2a>(
        stdx::ct_format<"Two runtime arguments: uint32_t {} and int64_t {}">(
            std::uint32_t{1}, std::int64_t{2}));
}
