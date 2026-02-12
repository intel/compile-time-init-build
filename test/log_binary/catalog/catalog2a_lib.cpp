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
auto log_ct_named_arg() -> void;
auto log_rt_named_arg() -> void;

auto log_two_rt_args() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2a>(
        stdx::ct_format<"Two runtime arguments: uint32_t {} and int64_t {}">(
            std::uint32_t{1}, std::int64_t{2}));
}

auto log_ct_named_arg() -> void {
    using namespace stdx::literals;
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2a>(
        stdx::ct_format<"One compile-time named argument: {foo}">("17"_ctst));
}

auto log_rt_named_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2a>(
        stdx::ct_format<"One runtime named argument: {foo}">(
            std::uint32_t{17}));
}
