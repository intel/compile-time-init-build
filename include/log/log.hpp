#pragma once

#include <log/env.hpp>
#include <log/level.hpp>
#include <log/module.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>
#include <stdx/utility.hpp>

#include <cstdint>
#include <utility>

namespace version {
namespace null {
struct config {
    constexpr static auto build_id = std::uint64_t{};
    constexpr static auto version_string = stdx::ct_string{""};
};
} // namespace null
template <typename...> inline auto config = null::config{};
} // namespace version

namespace logging {
namespace null {
struct config {
    struct {
        template <level, typename>
        constexpr auto log(auto &&...) const noexcept -> void {}
    } logger;
};
} // namespace null

template <typename...> inline auto config = null::config{};

struct default_flavor_t;

template <typename Flavor, typename... Ts>
ALWAYS_INLINE constexpr static auto get_config() -> auto & {
    if constexpr (std::same_as<Flavor, default_flavor_t>) {
        return config<Ts...>;
    } else {
        return config<Flavor, Ts...>;
    }
}

template <typename Flavor, level L, typename Env, typename... Ts,
          typename... TArgs>
ALWAYS_INLINE static auto log(TArgs &&...args) -> void {
    auto &cfg = get_config<Flavor, Ts...>();
    cfg.logger.template log<L, Env>(std::forward<TArgs>(args)...);
}
} // namespace logging

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CIB_LOG(FLAVOR, LEVEL, MSG, ...)                                       \
    logging::log<FLAVOR, LEVEL, cib_log_env_t>(                                \
        __FILE__, __LINE__, stdx::ct_format<MSG>(__VA_ARGS__))

#define CIB_TRACE(...)                                                         \
    CIB_LOG(logging::default_flavor_t, logging::level::TRACE, __VA_ARGS__)
#define CIB_INFO(...)                                                          \
    CIB_LOG(logging::default_flavor_t, logging::level::INFO, __VA_ARGS__)
#define CIB_WARN(...)                                                          \
    CIB_LOG(logging::default_flavor_t, logging::level::WARN, __VA_ARGS__)
#define CIB_ERROR(...)                                                         \
    CIB_LOG(logging::default_flavor_t, logging::level::ERROR, __VA_ARGS__)

#define CIB_FATAL(MSG, ...)                                                    \
    [](auto &&s) {                                                             \
        logging::log<logging::default_flavor_t, logging::level::FATAL,         \
                     cib_log_env_t>(__FILE__, __LINE__, s);                    \
        FWD(s).args.apply([](auto &&...args) {                                 \
            stdx::panic<decltype(s.str)::value>(FWD(args)...);                 \
        });                                                                    \
    }(stdx::ct_format<MSG>(__VA_ARGS__))

#define CIB_ASSERT(expr)                                                       \
    ((expr) ? void(0) : CIB_FATAL("Assertion failure: " #expr))

namespace logging {
template <typename Flavor, typename... Ts>
ALWAYS_INLINE static auto log_version() -> void {
    auto &l_cfg = get_config<Flavor, Ts...>();
    auto &v_cfg = ::version::config<Ts...>;
    if constexpr (requires {
                      l_cfg.logger.template log_build<v_cfg.build_id,
                                                      v_cfg.version_string>();
                  }) {
        l_cfg.logger.template log_build<v_cfg.build_id, v_cfg.version_string>();
    } else {
        l_cfg.logger.template log<level::MAX, cib_log_env_t>(
            "", 0,
            stdx::ct_format<"Version: {} ({})">(
                CX_VALUE(v_cfg.build_id), CX_VALUE(v_cfg.version_string)));
    }
}
} // namespace logging

#define CIB_LOG_V(FLAVOR) logging::log_version<FLAVOR>()
#define CIB_LOG_VERSION() CIB_LOG_V(logging::default_flavor_t)

// NOLINTEND(cppcoreguidelines-macro-usage)
