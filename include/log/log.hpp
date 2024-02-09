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
        template <level, typename>
        constexpr auto log(auto &&...) const noexcept -> void {}
    } logger;
};
} // namespace null

template <typename...> inline auto config = null::config{};

template <typename T>
concept loggable = requires(T const &t) {
    t.apply([]<typename StringType>(StringType, auto const &...) {});
};

template <level L, typename ModuleId, typename... Ts, typename... TArgs>
static auto log(TArgs &&...args) -> void {
    auto &cfg = config<Ts...>;
    cfg.logger.template log<L, ModuleId>(std::forward<TArgs>(args)...);
}

template <stdx::ct_string S> struct module_id_t {
    using type = decltype(stdx::ct_string_to_type<S, sc::string_constant>());
};
} // namespace logging

using cib_log_module_id_t = typename logging::module_id_t<"default">::type;

#define CIB_DO_PRAGMA(X) _Pragma(#X)
#ifdef __clang__
#define CIB_PRAGMA(X) CIB_DO_PRAGMA(clang X)
#define CIB_PRAGMA_SEMI
#else
#define CIB_PRAGMA(X) CIB_DO_PRAGMA(GCC X)
#define CIB_PRAGMA_SEMI ;
#endif

#define CIB_LOG_MODULE(S)                                                      \
    CIB_PRAGMA(diagnostic push)                                                \
    CIB_PRAGMA(diagnostic ignored "-Wshadow")                                  \
    using cib_log_module_id_t [[maybe_unused]] =                               \
        typename logging::module_id_t<S>::type CIB_PRAGMA_SEMI CIB_PRAGMA(     \
            diagnostic pop)

#define CIB_LOG(LEVEL, MSG, ...)                                               \
    logging::log<LEVEL, cib_log_module_id_t>(                                  \
        __FILE__, __LINE__, sc::formatter{MSG##_sc}(__VA_ARGS__))

#define CIB_TRACE(...) CIB_LOG(logging::level::TRACE, __VA_ARGS__)
#define CIB_INFO(...) CIB_LOG(logging::level::INFO, __VA_ARGS__)
#define CIB_WARN(...) CIB_LOG(logging::level::WARN, __VA_ARGS__)
#define CIB_ERROR(...) CIB_LOG(logging::level::ERROR, __VA_ARGS__)
#define CIB_FATAL(...)                                                         \
    (CIB_LOG(logging::level::FATAL, __VA_ARGS__), STDX_PANIC(__VA_ARGS__))

#define CIB_ASSERT(expr)                                                       \
    ((expr) ? void(0) : CIB_FATAL("Assertion failure: " #expr))
