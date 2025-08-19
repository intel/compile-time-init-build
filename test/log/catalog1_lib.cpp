#include "catalog_concurrency.hpp"

#include <log/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>

#include <conc/concurrency.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

int log_calls{};
std::uint32_t last_header{};

namespace {
struct test_log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        ++log_calls;
        last_header = pkt[0];
    }
};

using log_env1 = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_zero_args() -> void;
auto log_one_ct_arg() -> void;
auto log_one_32bit_rt_arg() -> void;
auto log_one_64bit_rt_arg() -> void;
auto log_one_formatted_rt_arg() -> void;
auto log_with_non_default_module() -> void;
auto log_with_fixed_module() -> void;
auto log_with_fixed_string_id() -> void;
auto log_with_fixed_module_id() -> void;

auto log_zero_args() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"A string with no placeholders">());
}

auto log_one_ct_arg() -> void {
    using namespace stdx::literals;
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"B string with {} placeholder">("one"_ctst));
}

auto log_one_32bit_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"C1 string with {} placeholder">(std::int32_t{1}));
}

auto log_one_64bit_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"C2 string with {} placeholder">(std::int64_t{1}));
}

auto log_one_formatted_rt_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    cfg.logger.log_msg<log_env1>(
        stdx::ct_format<"C3 string with {:08x} placeholder">(std::int32_t{1}));
}

auto log_with_non_default_module() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "not default") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"ModuleID string with {} placeholder">(1));
    }
}

auto log_with_fixed_module() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module, "fixed") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed ModuleID string with {} placeholder">(1));
    }
}

auto log_with_fixed_string_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_string_id, 1337) {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed StringID string">());
    }
}

auto log_with_fixed_module_id() -> void {
    CIB_WITH_LOG_ENV(logging::get_level, logging::level::TRACE,
                     logging::get_module_id, 7, logging::get_module,
                     "fixed_id") {
        auto cfg = logging::binary::config{test_log_destination{}};
        cfg.logger.log_msg<cib_log_env_t>(
            stdx::ct_format<"Fixed ModuleID string with {} placeholder">(1));
    }
}
