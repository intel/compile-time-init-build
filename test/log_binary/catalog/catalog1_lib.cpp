#include "catalog_concurrency.hpp"
#include "catalog_destination.hpp"

#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>

#include <cstdint>

namespace {
using log_env1 = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_zero_args() -> void;
auto log_one_ct_arg() -> void;
auto log_one_32bit_rt_arg() -> void;
auto log_one_64bit_rt_arg() -> void;
auto log_one_formatted_rt_arg() -> void;
auto log_with_default_module() -> void;
auto log_with_non_default_module() -> void;
auto log_with_fixed_module() -> void;
auto log_with_fixed_string_id() -> void;
auto log_with_fixed_unsigned_string_id() -> void;
auto log_with_fixed_module_id() -> void;
auto log_with_fixed_unsigned_module_id() -> void;

auto log_zero_args() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(stdx::ct_format<"Zero arguments">());
}

auto log_one_ct_arg() -> void {
    using namespace stdx::literals;
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"One compile-time argument: {}">("17"_ctst));
}

auto log_one_32bit_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"One int32_t runtime argument: {}">(std::int32_t{17}));
}

auto log_one_64bit_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"One int64_t runtime argument: {}">(std::int64_t{17}));
}

auto log_one_formatted_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<
            "One int32_t runtime argument formatted as {{:08x}}: {:08x}">(
            std::int32_t{17}));
}

auto log_with_default_module() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"Default module with runtime argument: {}">(17));
}

auto log_with_non_default_module() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "not default") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Overridden module (\"not default\") with runtime "
                            "argument: {}">(17));
    }
}

auto log_with_fixed_module() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "fixed") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<
                "Fixed module (\"fixed\") with runtime argument: {}">(17));
    }
}

auto log_with_fixed_string_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_string_id, 1337) {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed string_id (1337)">());
    }
}

auto log_with_fixed_unsigned_string_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_string_id, 1338u) {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed unsigned string_id (1338)">());
    }
}

auto log_with_fixed_module_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module_id, 6, logging::get_module,
                     "fixed_id6") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed module_id (6) and module (\"fixed_id6\") "
                            "with runtime argument: {}">(17));
    }
}

auto log_with_fixed_unsigned_module_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module_id, 7ULL, logging::get_module,
                     "fixed_id7") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<
                "Fixed unsigned module_id (7) and module (\"fixed_id7\") "
                "with runtime argument: {}">(17));
    }
}
