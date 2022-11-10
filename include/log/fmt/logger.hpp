#pragma once

#define FMT_HEADER_ONLY
#include <log/fmt/catalog.hpp>
#include <log/log_level.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <cctype>
#include <chrono>
#include <fmt/format.h>

template <> struct fmt::formatter<log_level> {
    constexpr static auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const log_level &level, FormatContext &ctx) {
        switch (level) {
        case log_level::TRACE:
            return format_to(ctx.out(), "TRACE");

        case log_level::INFO:
            return format_to(ctx.out(), "INFO");

        case log_level::WARN:
            return format_to(ctx.out(), "WARN");

        case log_level::ERROR:
            return format_to(ctx.out(), "ERROR");

        case log_level::FATAL:
            return format_to(ctx.out(), "FATAL");

        default:
            return format_to(ctx.out(), "UNKNOWN");
        }
    }
};

namespace {
inline const auto loggingStartTime = std::chrono::high_resolution_clock::now();

template <typename StringType>
inline auto trimSourceFilename(StringType src) -> char const * {
    return std::strstr(src, "src");
}

template <typename T> struct FormatHelper {
    constexpr static T str{};

    constexpr explicit FormatHelper(T) {}

    template <typename... Ts> constexpr static auto f(Ts... args) {
        return format(str, args...);
    }
};

inline auto outputLine = []([[maybe_unused]] auto filename,
                            [[maybe_unused]] auto lineNumber, auto level,
                            auto msg) {
    auto currentTime =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - loggingStartTime)
            .count();

    fmt::print("{:>8}us {}: {}\n", currentTime, level, msg);
};

template <typename FilenameStringType, typename LineNumberType,
          typename MsgType>
void log(FilenameStringType filename, LineNumberType lineNumber,
         log_level level, MsgType msg) {
    auto formattedMsg = msg.args.apply(
        [&](auto... args) { return fmt::format(msg.str.value, args...); });

    outputLine(filename, lineNumber, level, formattedMsg);
}
} // namespace

#define CIB_LOG(LEVEL, MSG, ...)                                               \
    log(__FILE__, __LINE__, LEVEL, FormatHelper{MSG##_sc}.f(__VA_ARGS__))

#define CIB_TRACE(...) CIB_LOG(log_level::TRACE, __VA_ARGS__)
#define CIB_INFO(...) CIB_LOG(log_level::INFO, __VA_ARGS__)
#define CIB_WARN(...) CIB_LOG(log_level::WARN, __VA_ARGS__)
#define CIB_ERROR(...) CIB_LOG(log_level::ERROR, __VA_ARGS__)
#define CIB_FATAL(...) CIB_LOG(log_level::FATAL, __VA_ARGS__)

#define CIB_ASSERT(expr)                                                       \
    ((expr) ? void(0) : CIB_FATAL("Assertion failure:  #expr"))
