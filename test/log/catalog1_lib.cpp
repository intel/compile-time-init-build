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
} // namespace

auto log_zero_args() -> void;
auto log_one_ct_arg() -> void;
auto log_one_rt_arg() -> void;
auto log_with_non_default_module_id() -> void;
auto log_with_fixed_module_id() -> void;

auto log_zero_args() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        "A string with no placeholders"_sc);
}

auto log_one_ct_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("B string with {} placeholder"_sc, "one"_sc));
}

auto log_one_rt_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("C string with {} placeholder"_sc, 1));
}

auto log_with_non_default_module_id() -> void {
    CIB_LOG_MODULE("not default");
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("ModuleID string with {} placeholder"_sc, 1));
}

auto log_with_fixed_module_id() -> void {
    CIB_LOG_MODULE("fixed");
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("Fixed ModuleID string with {} placeholder"_sc, 1));
}
