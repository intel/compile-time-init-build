#pragma once

#include <log/level.hpp>
#include <log/log.hpp>
#include <log/module.hpp>

#include <stdx/bit.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <fmt/format.h>

#include <array>
#include <chrono>
#include <iterator>
#include <string_view>
#include <utility>

namespace logging {
template <auto> struct level_wrapper {};

namespace fmt_detail {
using namespace std::string_view_literals;
constexpr std::array level_text{"MAX"sv,  "FATAL"sv, "ERROR"sv, "WARN"sv,
                                "INFO"sv, "USER1"sv, "USER2"sv, "TRACE"sv};
} // namespace fmt_detail

template <logging::level L>
constexpr std::string_view level_text =
    fmt_detail::level_text[stdx::to_underlying(L)];

template <logging::level L>
[[nodiscard]] constexpr auto format_as(level_wrapper<L>) -> std::string_view {
    return level_text<L>;
}

namespace fmt {
template <typename TDestinations> struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    auto log(FilenameStringType, LineNumberType, MsgType const &msg) -> void {
        auto const currentTime =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start_time)
                .count();

        stdx::for_each(
            [&](auto &out) {
                ::fmt::format_to(out, "{:>8}us {} [{}]: ", currentTime,
                                 level_wrapper<get_level(Env{})>{},
                                 get_module(Env{}));
                msg.apply(
                    [&]<typename StringType>(StringType, auto const &...args) {
                        ::fmt::format_to(out, StringType::value, args...);
                    });
                *out = '\n';
            },
            dests);
    }

  private:
    static inline auto const start_time = std::chrono::steady_clock::now();
    TDestinations dests;
};

template <typename... TDestinations> struct config {
    using destinations_tuple_t = stdx::tuple<TDestinations...>;
    constexpr explicit config(TDestinations... dests)
        : logger{stdx::tuple{std::move(dests)...}} {}

    log_handler<destinations_tuple_t> logger;
};
} // namespace fmt
} // namespace logging
