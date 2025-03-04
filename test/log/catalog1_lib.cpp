#include "catalog_concurrency.hpp"

#include <conc/concurrency.hpp>
#include <log/catalog/mipi_encoder.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

int log_calls{};
std::uint32_t last_header{};

namespace {
struct test_log_args_destination {
    auto log_by_args(std::uint32_t hdr, auto...) -> void {
        ++log_calls;
        last_header = hdr;
    }
};

using log_env1 = logging::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_zero_args() -> void;
auto log_one_ct_arg() -> void;
auto log_one_32bit_rt_arg() -> void;
auto log_one_64bit_rt_arg() -> void;
auto log_one_formatted_rt_arg() -> void;
auto log_with_non_default_module_id() -> void;
auto log_with_fixed_module_id() -> void;

auto log_zero_args() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env1>("A string with no placeholders"_sc);
}

auto log_one_ct_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env1>(
        format("B string with {} placeholder"_sc, "one"_sc));
}

auto log_one_32bit_rt_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env1>(
        format("C1 string with {} placeholder"_sc, std::int32_t{1}));
}

auto log_one_64bit_rt_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env1>(
        format("C2 string with {} placeholder"_sc, std::int64_t{1}));
}

auto log_one_formatted_rt_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<log_env1>(
        format("C3 string with {:08x} placeholder"_sc, std::int32_t{1}));
}

auto log_with_non_default_module_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "not default") {
        auto cfg = logging::mipi::config{test_log_args_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            format("ModuleID string with {} placeholder"_sc, 1));
    }
}

auto log_with_fixed_module_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "fixed") {
        auto cfg = logging::mipi::config{test_log_args_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            format("Fixed ModuleID string with {} placeholder"_sc, 1));
    }
}
