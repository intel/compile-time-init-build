#include "catalog_concurrency.hpp"

#include <conc/concurrency.hpp>
#include <log/catalog/mipi_encoder.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

int log_calls{};

namespace {
struct test_log_args_destination {
    auto log_by_args(std::uint32_t, auto...) -> void { ++log_calls; }
};
} // namespace

auto log_zero_args() -> void;
auto log_one_ct_arg() -> void;
auto log_one_rt_arg() -> void;

auto log_zero_args() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_module_id_t>(
        "A string with no placeholders"_sc);
}

auto log_one_ct_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_module_id_t>(
        format("B string with {} placeholder"_sc, "one"_sc));
}

auto log_one_rt_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_module_id_t>(
        format("C string with {} placeholder"_sc, 1));
}
