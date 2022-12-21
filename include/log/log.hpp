#pragma once

#include <log/level.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <utility>

namespace logging {
namespace null {
struct config {
    struct {
        template <level L, typename... Ts>
        constexpr auto log(Ts &&...) const noexcept -> void {}
    } logger;

    static constexpr auto terminate() noexcept -> void {}
};
} // namespace null

template <typename...> inline auto config = null::config{};

template <level L, typename... Ts, typename... TArgs>
static auto log(TArgs &&...args) -> void {
    auto &cfg = config<Ts...>;
    cfg.logger.template log<L>(std::forward<TArgs>(args)...);
}

template <typename... Ts> static auto terminate() -> void {
    auto &cfg = config<Ts...>;
    cfg.terminate();
}
} // namespace logging

#define CIB_LOG(LEVEL, MSG, ...)                                               \
    logging::log<LEVEL>(__FILE__, __LINE__,                                    \
                        sc::formatter{MSG##_sc}(__VA_ARGS__))

#define CIB_TRACE(...) CIB_LOG(logging::level::TRACE, __VA_ARGS__)
#define CIB_INFO(...) CIB_LOG(logging::level::INFO, __VA_ARGS__)
#define CIB_WARN(...) CIB_LOG(logging::level::WARN, __VA_ARGS__)
#define CIB_ERROR(...) CIB_LOG(logging::level::ERROR, __VA_ARGS__)
#define CIB_FATAL(...)                                                         \
    (CIB_LOG(logging::level::FATAL, __VA_ARGS__), logging::terminate())

#define CIB_ASSERT(expr)                                                       \
    ((expr) ? void(0) : CIB_FATAL("Assertion failure: " #expr))

#include <log/catalog/mipi_encoder.hpp>
#include <log/fmt/logger.hpp>
