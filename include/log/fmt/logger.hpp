#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>


#include <chrono>
#include <cctype>

#include <sc/string_constant.hpp>
#include <sc/format.hpp>
#include <log/fmt/catalog.hpp>
#include <log/log_level.hpp>

template <>
struct fmt::formatter<log_level> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    template<typename FormatContext>
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
    const auto loggingStartTime = std::chrono::high_resolution_clock::now();

    template<typename StringType>
    inline char const * trimSourceFilename(StringType src) {
        return std::strstr(src, "src");
    }

    template<typename T>
    struct FormatHelper {
        constexpr static T str{};

        constexpr FormatHelper(T) {}

        template<typename... Ts>
        constexpr static auto f(Ts... args) {
            return format(str, args...);
        }
    };


    auto outputLine = [](auto filename, auto lineNumber, auto level, auto msg){
        auto currentTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - loggingStartTime).count();

        fmt::print(
            "{:>8}us {}: {}\n",
            currentTime,
            level,
            msg);
    };

    template<
        typename FilenameStringType,
        typename LineNumberType,
        typename MsgType>
    void log(
        FilenameStringType filename,
        LineNumberType lineNumber,
        log_level level,
        MsgType msg
    ) {
        auto formattedMsg = msg.args.apply([&](auto... args){
            return fmt::format(msg.str.value, args...);
        });

        outputLine(filename, lineNumber, level, formattedMsg);
    }
}

#define LOG(LEVEL, MSG, ...) log(__FILE__, __LINE__, LEVEL, FormatHelper{MSG ## _sc}.f(__VA_ARGS__))

#define TRACE(...) LOG(log_level::TRACE, __VA_ARGS__)
#define INFO(...) LOG(log_level::INFO, __VA_ARGS__)
#define WARN(...) LOG(log_level::WARN, __VA_ARGS__)
#define ERROR(...) LOG(log_level::ERROR, __VA_ARGS__)
#define FATAL(...) LOG(log_level::FATAL, __VA_ARGS__)

#define ASSERT(expr) (expr ? void(0) : FATAL("Assertion failure:  #expr"))