#pragma once

#include <log/env.hpp>
#include <log/flavor.hpp>
#include <log/level.hpp>
#include <log/module.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>
#include <stdx/pp_map.hpp>
#if __cpp_pack_indexing < 202311L
#include <stdx/tuple.hpp>
#endif
#include <stdx/type_traits.hpp>
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
        template <typename>
        constexpr auto log(auto &&...) const noexcept -> void {}
    } logger;
};
} // namespace null

template <typename...> inline auto config = null::config{};

template <typename Env, typename... Ts>
constexpr static auto get_config() -> auto & {
    using flavor_t = typename decltype(get_flavor(Env{}))::type;
    if constexpr (std::same_as<flavor_t, default_flavor_t>) {
        return config<Ts...>;
    } else {
        return config<flavor_t, Ts...>;
    }
}

template <typename Env, typename... Ts, typename... TArgs>
static auto log(TArgs &&...args) -> void {
    auto &cfg = get_config<Env, Ts...>();
    cfg.logger.template log<Env>(std::forward<TArgs>(args)...);
}

namespace detail {
template <typename> constexpr auto is_already_ct = false;
template <typename T, T V>
constexpr auto is_already_ct<std::integral_constant<T, V>> = true;
template <stdx::ct_string S>
constexpr auto is_already_ct<stdx::cts_t<S>> = true;
template <typename T>
constexpr auto is_already_ct<stdx::type_identity<T>> = true;

template <stdx::ct_string Msg> constexpr auto cx_log_wrap(int, auto &&...args) {
    return stdx::ct_format<Msg>(FWD(args)...);
}
} // namespace detail
} // namespace logging

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CIB_LOG(...)                                                           \
    logging::log<cib_log_env_t>(__FILE__, __LINE__, STDX_CT_FORMAT(__VA_ARGS__))

#define CIB_LOG_WITH_LEVEL(LEVEL, ...)                                         \
    logging::log<                                                              \
        stdx::extend_env_t<cib_log_env_t, logging::get_level, LEVEL>>(         \
        __FILE__, __LINE__, STDX_CT_FORMAT(__VA_ARGS__))

#define CIB_TRACE(...)                                                         \
    CIB_LOG_WITH_LEVEL(logging::level::TRACE __VA_OPT__(, ) __VA_ARGS__)
#define CIB_INFO(...)                                                          \
    CIB_LOG_WITH_LEVEL(logging::level::INFO __VA_OPT__(, ) __VA_ARGS__)
#define CIB_WARN(...)                                                          \
    CIB_LOG_WITH_LEVEL(logging::level::WARN __VA_OPT__(, ) __VA_ARGS__)
#define CIB_ERROR(...)                                                         \
    CIB_LOG_WITH_LEVEL(logging::level::ERROR __VA_OPT__(, ) __VA_ARGS__)

namespace logging {
template <stdx::ct_string Fmt, typename Env, typename F, typename L>
[[nodiscard]] constexpr auto panic(F file, L line, auto &&...args) {
    STDX_PRAGMA(diagnostic push)
#ifdef __clang__
    STDX_PRAGMA(diagnostic ignored "-Wunknown-warning-option")
    STDX_PRAGMA(diagnostic ignored "-Wc++26-extensions")
#endif

    constexpr auto N = stdx::num_fmt_specifiers<Fmt>;
    constexpr auto sz = sizeof...(args);

#if __cpp_pack_indexing >= 202311L
    auto s = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return stdx::ct_format<Fmt>(FWD(args...[Is])...);
    }(std::make_index_sequence<N>{});
    log<Env>(file, line, s);

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        stdx::panic<decltype(s.str)::value>(std::move(s).args,
                                            FWD(args...[N + Is])...);
    }(std::make_index_sequence<sz - N>{});
#else
    auto tup = stdx::make_tuple(FWD(args)...);
    auto t = [&]<std::size_t... Is, std::size_t... Js>(
                 std::index_sequence<Is...>, std::index_sequence<Js...>) {
        return stdx::make_tuple(
            stdx::make_tuple(stdx::get<Is>(std::move(tup))...),
            stdx::make_tuple(stdx::get<sizeof...(Is) + Js>(std::move(tup))...));
    }(std::make_index_sequence<N>{}, std::make_index_sequence<sz - N>{});

    auto s = stdx::get<0>(std::move(t)).apply([](auto &&...fmt_args) {
        return stdx::ct_format<Fmt>(FWD(fmt_args)...);
    });
    log<Env>(file, line, s);

    stdx::get<1>(std::move(t)).apply([&](auto &&...extra_args) {
        stdx::panic<decltype(s.str)::value>(std::move(s).args,
                                            FWD(extra_args)...);
    });
#endif

    STDX_PRAGMA(diagnostic pop)
}
} // namespace logging

#define CIB_FATAL(MSG, ...)                                                    \
    logging::panic<MSG, stdx::extend_env_t<cib_log_env_t, logging::get_level,  \
                                           logging::level::FATAL>>(            \
        __FILE__, __LINE__ __VA_OPT__(, STDX_MAP(CX_WRAP, __VA_ARGS__)))

#define CIB_ASSERT(expr, ...)                                                  \
    ((expr)                                                                    \
         ? void(0)                                                             \
         : CIB_FATAL("Assertion failure: " #expr __VA_OPT__(, ) __VA_ARGS__))

namespace logging {
template <typename Env, typename... Ts> static auto log_version() -> void {
    auto &l_cfg = get_config<Env, Ts...>();
    auto &v_cfg = ::version::config<Ts...>;
    if constexpr (requires {
                      l_cfg.logger.template log_version<Env, v_cfg.build_id,
                                                        v_cfg.version_string>();
                  }) {
        l_cfg.logger
            .template log_version<Env, v_cfg.build_id, v_cfg.version_string>();
    } else {
        CIB_LOG_ENV(logging::get_level, logging::level::MAX);
        l_cfg.logger.template log<cib_log_env_t>(
            "", 0,
            stdx::ct_format<"Version: {} ({})">(
                CX_VALUE(v_cfg.build_id), CX_VALUE(v_cfg.version_string)));
    }
}
} // namespace logging

#define CIB_LOG_V(FLAVOR)                                                      \
    logging::log_version<stdx::extend_env_t<                                   \
        cib_log_env_t, logging::get_flavor, stdx::type_identity<FLAVOR>{}>>()
#define CIB_LOG_VERSION() CIB_LOG_V(logging::default_flavor_t)

// NOLINTEND(cppcoreguidelines-macro-usage)
