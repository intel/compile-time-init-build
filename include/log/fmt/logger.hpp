#pragma once

#include <cib/tuple.hpp>
#include <log/log.hpp>

#include <fmt/format.h>

#include <chrono>
#include <exception>
#include <iostream>
#include <iterator>
#include <utility>

template <auto L> struct fmt::formatter<logging::level_constant<L>> {
    constexpr static auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(logging::level_constant<L>, FormatContext &ctx) {
        return ::fmt::format_to(ctx.out(), to_text(L));
    }
};

namespace logging::fmt {
template <typename TDestinations> struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <logging::level L, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    auto log(FilenameStringType, LineNumberType, MsgType const &msg) -> void {
        auto const currentTime =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start_time)
                .count();

        cib::for_each(
            [&](auto &out) {
                ::fmt::format_to(out, "{:>8}us {}: ", currentTime,
                                 level_constant<L>{});
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
    using destinations_tuple_t = cib::tuple<TDestinations...>;
    constexpr explicit config(TDestinations... dests)
        : logger{cib::tuple{std::move(dests)...}} {}

    log_handler<destinations_tuple_t> logger;

    [[noreturn]] static auto terminate() { std::terminate(); }
};
} // namespace logging::fmt
