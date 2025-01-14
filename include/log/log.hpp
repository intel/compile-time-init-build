#pragma once

#include <log/env.hpp>
#include <log/level.hpp>
#include <log/module.hpp>
#include <sc/format.hpp>
#include <sc/fwd.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

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
        __FILE__, __LINE__, sc::format(MSG##_sc __VA_OPT__(, ) __VA_ARGS__))

#define CIB_TRACE(...)                                                         \
    CIB_LOG(logging::default_flavor_t, logging::level::TRACE, __VA_ARGS__)
#define CIB_INFO(...)                                                          \
    CIB_LOG(logging::default_flavor_t, logging::level::INFO, __VA_ARGS__)
#define CIB_WARN(...)                                                          \
    CIB_LOG(logging::default_flavor_t, logging::level::WARN, __VA_ARGS__)
#define CIB_ERROR(...)                                                         \
    CIB_LOG(logging::default_flavor_t, logging::level::ERROR, __VA_ARGS__)

#define CIB_FATAL(MSG, ...)                                                    \
    [](auto &&str) {                                                           \
        logging::log<logging::default_flavor_t, logging::level::FATAL,         \
                     cib_log_env_t>(__FILE__, __LINE__, str);                  \
        FWD(str).apply([]<typename S, typename... Args>(S s, Args... args) {   \
            constexpr auto cts = stdx::ct_string_from_type(s);                 \
            stdx::panic<cts>(args...);                                         \
        });                                                                    \
    }(sc::format(MSG##_sc __VA_OPT__(, ) __VA_ARGS__))

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
            sc::format("Version: {} ({})"_sc, sc::uint_<v_cfg.build_id>,
                       stdx::ct_string_to_type<v_cfg.version_string,
                                               sc::string_constant>()));
    }
}
} // namespace logging

#define CIB_LOG_V(FLAVOR) logging::log_version<FLAVOR>()
#define CIB_LOG_VERSION() CIB_LOG_V(logging::default_flavor_t)

// NOLINTEND(cppcoreguidelines-macro-usage)
