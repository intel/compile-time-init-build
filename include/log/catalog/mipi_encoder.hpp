#pragma once

#include <log/catalog/catalog.hpp>
#include <log/log.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>

#include <cstdint>
#include <exception>
#include <utility>

namespace {
template <logging::level L, typename S, typename T>
constexpr auto to_message() {
    constexpr auto s = S::value;
    using char_t = typename std::remove_cv_t<decltype(s)>::value_type;
    return [&]<template <typename...> typename Tuple, typename... Args,
               std::size_t... Is>(Tuple<Args...> const &,
                                  std::integer_sequence<std::size_t, Is...>) {
        return message<L, sc::undefined<sc::args<Args...>, char_t, s[Is]...>>{};
    }(T{}, std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <logging::level L, typename Msg> constexpr auto to_message(Msg msg) {
    if constexpr (requires { msg.args; }) {
        return to_message<L, decltype(msg.str), decltype(msg.args)>();
    } else {
        return to_message<L, Msg, stdx::tuple<>>();
    }
}
} // namespace

namespace logging::mipi {
template <typename ConcurrencyPolicy, typename TDestinations>
struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <logging::level Level, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    ALWAYS_INLINE auto log(FilenameStringType, LineNumberType,
                           MsgType const &msg) -> void {
        log_msg<Level>(msg);
    }

    ALWAYS_INLINE auto log_id(string_id id) -> void {
        dispatch_message<logging::level::TRACE>(id);
    }

    template <logging::level Level, typename Msg>
    ALWAYS_INLINE auto log_msg(Msg msg) -> void {
        msg.apply([&]<typename StringType>(StringType, auto... args) {
            using Message = decltype(to_message<Level>(msg));
            dispatch_message<Level>(catalog<Message>(),
                                    static_cast<std::uint32_t>(args)...);
        });
    }

  private:
    CONSTEVAL static auto make_catalog32_header(logging::level level)
        -> std::uint32_t {
        return (0x1u << 24u) | // mipi sys-t subtype: id32_p32
               (static_cast<std::uint32_t>(level) << 4u) |
               0x3u; // mipi sys-t type: catalog
    }

    constexpr static auto make_short32_header(string_id id) -> std::uint32_t {
        return (id << 4u) | 1u;
    }

    template <typename... MsgDataTypes>
    NEVER_INLINE auto dispatch_pass_by_args(MsgDataTypes &&...msg_data)
        -> void {
        ConcurrencyPolicy::call_in_critical_section([&] {
            stdx::for_each(
                [&](auto &dest) {
                    dest.log_by_args(std::forward<MsgDataTypes>(msg_data)...);
                },
                dests);
        });
    }

    NEVER_INLINE auto dispatch_pass_by_buffer(std::uint32_t *msg,
                                              std::uint32_t msg_size) -> void {
        ConcurrencyPolicy::call_in_critical_section([&] {
            stdx::for_each([&](auto &dest) { dest.log_by_buf(msg, msg_size); },
                           dests);
        });
    }

    template <logging::level Level, typename... MsgDataTypes>
    ALWAYS_INLINE auto dispatch_message(string_id id,
                                        MsgDataTypes &&...msg_data) -> void {
        if constexpr (sizeof...(msg_data) == 0u) {
            dispatch_pass_by_args(make_short32_header(id));
        } else if constexpr (sizeof...(msg_data) <= 2u) {
            dispatch_pass_by_args(make_catalog32_header(Level), id,
                                  std::forward<MsgDataTypes>(msg_data)...);
        } else {
            std::array args = {make_catalog32_header(Level), id,
                               std::forward<MsgDataTypes>(msg_data)...};
            dispatch_pass_by_buffer(args.data(), args.size());
        }
    }

    TDestinations dests;
};

template <typename ConcurrencyPolicy> struct under {
    template <typename... TDestinations> struct config {
        using destinations_tuple_t = stdx::tuple<TDestinations...>;
        constexpr explicit config(TDestinations... dests)
            : logger{stdx::tuple{std::move(dests)...}} {}

        log_handler<ConcurrencyPolicy, destinations_tuple_t> logger;

        [[noreturn]] static auto terminate() { std::terminate(); }
    };

    // Clang needs a deduction guide here. GCC does not, and in fact GCC
    // has a bug: it claims that deduction guides must be at namespace
    // scope.
#ifdef __clang__
    template <typename... Ts> config(Ts...) -> config<Ts...>;
#endif
};
} // namespace logging::mipi
