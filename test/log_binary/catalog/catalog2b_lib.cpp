#include "catalog_concurrency.hpp"
#include "catalog_destination.hpp"
#include "catalog_enums.hpp"

#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>

#include <cstdint>

namespace {
using log_env2b = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_rt_scoped_enum_arg() -> void;
auto log_rt_unscoped_enum_arg() -> void;
auto log_rt_auto_scoped_enum_arg() -> void;
auto log_rt_float_arg() -> void;
auto log_rt_double_arg() -> void;

auto log_rt_scoped_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"Scoped enum argument: {}">(E1::VAL_E1));
}

auto log_rt_unscoped_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"Unscoped enum argument: {}">(VAL_E2));
}

namespace some_ns {
enum struct E { A = 17, B = 18, C = 19 };
}

auto log_rt_auto_scoped_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace some_ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"Auto-declared scoped enum argument: {}">(E::A));
}

auto log_rt_float_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2b>(stdx::ct_format<"Float argument: {}">(3.14f));
}

auto log_rt_double_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env2b>(stdx::ct_format<"Double argument: {}">(3.14));
}
