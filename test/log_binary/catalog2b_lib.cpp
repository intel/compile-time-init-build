#include "catalog_concurrency.hpp"
#include "catalog_enums.hpp"

#include <log_binary/catalog/encoder.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>

#include <conc/concurrency.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;

namespace {
struct test_log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N>) const {
        ++log_calls;
    }
};

using log_env2b = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

auto log_rt_enum_arg() -> void;
auto log_rt_auto_scoped_enum_arg() -> void;
auto log_rt_float_arg() -> void;
auto log_rt_double_arg() -> void;

auto log_rt_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"E string with {} placeholder">(VAL));
}

namespace some_ns {
enum struct E { A, B, C };
}

auto log_rt_auto_scoped_enum_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace some_ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"E (scoped) string with {} placeholder">(E::A));
}

auto log_rt_float_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"Float string with {} placeholder">(3.14f));
}

auto log_rt_double_arg() -> void {
    auto cfg = logging::binary::config{test_log_destination{}};
    using namespace ns;
    cfg.logger.log_msg<log_env2b>(
        stdx::ct_format<"Double string with {} placeholder">(3.14));
}
