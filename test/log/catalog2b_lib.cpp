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
};
} // namespace

auto log_rt_enum_arg() -> void;

auto log_rt_enum_arg() -> void {
    auto cfg = logging::mipi::config{test_log_args_destination{}};
    using namespace ns;
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("E string with {} placeholder"_sc, E::value));
}
