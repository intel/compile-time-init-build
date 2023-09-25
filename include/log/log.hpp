#pragma once

#include <log/level.hpp>
#include <sc/format.hpp>
#include <sc/fwd.hpp>

#include <stdx/panic.hpp>

#include <utility>

namespace logging {
namespace null {
struct config {
    struct {
        template <level L, typename... Ts>
        // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
        constexpr auto log(Ts &&...) const noexcept -> void {}
    } logger;
};
} // namespace null

template <typename...> inline auto config = null::config{};

template <level L, typename... Ts, typename... TArgs>
static auto log(TArgs &&...args) -> void {
    auto &cfg = config<Ts...>;
    cfg.logger.template log<L>(std::forward<TArgs>(args)...);
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
    (CIB_LOG(logging::level::FATAL, __VA_ARGS__), STDX_PANIC(__VA_ARGS__))

#define CIB_ASSERT(expr)                                                       \
    ((expr) ? void(0) : CIB_FATAL("Assertion failure: " #expr))
