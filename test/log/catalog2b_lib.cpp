#include "catalog_concurrency.hpp"
#include "catalog_enums.hpp"

#include <log/catalog/encoder.hpp>

#include <conc/concurrency.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;

namespace {
struct test_log_args_destination {
    auto log_by_args(std::uint32_t, auto...) -> void { ++log_calls; }
};

using log_env2b = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_rt_enum_arg() -> void;

auto log_rt_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_args_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        format("E string with {} placeholder"_sc, E::value));
}
