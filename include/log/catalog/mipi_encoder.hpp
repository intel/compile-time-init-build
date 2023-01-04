#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/tuple.hpp>
#include <log/catalog/catalog.hpp>
#include <log/log.hpp>

#include <cstdint>
#include <exception>
#include <utility>

namespace logging::mipi {
template <typename TCriticalSection, typename TDestinations>
struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <logging::level Level, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    CIB_ALWAYS_INLINE auto log(FilenameStringType, LineNumberType,
                               MsgType const &msg) -> void {
        log_msg<Level>(msg);
    }

    CIB_ALWAYS_INLINE auto log_id(string_id id) -> void {
        dispatch_message<logging::level::TRACE>(id);
    }

    template <logging::level Level, typename StringType>
    CIB_ALWAYS_INLINE auto log_msg(StringType msg) -> void {
        msg.args.apply([&](auto... args) {
            using Message = message<Level, StringType>;
            dispatch_message<Level>(catalog<Message>(),
                                    static_cast<std::uint32_t>(args)...);
        });
    }

  private:
    CIB_CONSTEVAL static auto make_catalog32_header(logging::level level)
        -> std::uint32_t {
        return (0x1u << 24u) | // mipi sys-t subtype: id32_p32
               (static_cast<std::uint32_t>(level) << 4u) |
               0x3u; // mipi sys-t type: catalog
    }

    constexpr static auto make_short32_header(string_id id) -> std::uint32_t {
        return (id << 4u) | 1u;
    }

    template <typename... MsgDataTypes>
    CIB_NEVER_INLINE auto dispatch_pass_by_args(MsgDataTypes... msg_data)
        -> void {
        TCriticalSection cs{};
        cib::for_each([&](auto &dest) { dest.log_by_args(msg_data...); },
                      dests);
    }

    CIB_NEVER_INLINE auto dispatch_pass_by_buffer(std::uint32_t *msg,
                                                  std::uint32_t msg_size)
        -> void {
        TCriticalSection cs{};
        cib::for_each([&](auto &dest) { dest.log_by_buf(msg, msg_size); },
                      dests);
    }

    template <logging::level Level, typename... MsgDataTypes>
    CIB_ALWAYS_INLINE auto dispatch_message(string_id id,
                                            MsgDataTypes &&...msg_data)
        -> void {
        if constexpr (sizeof...(msg_data) == 0u) {
            dispatch_pass_by_args(make_short32_header(id));
        } else if constexpr (sizeof...(msg_data) <= 2u) {
            dispatch_pass_by_args(make_catalog32_header(Level), id,
                                  msg_data...);
        } else {
            std::array args = {make_catalog32_header(Level), id, msg_data...};
            dispatch_pass_by_buffer(args.data(), args.size());
        }
    }

    TDestinations dests;
};

template <typename TCriticalSection> struct under {
    template <typename... TDestinations> struct config {
        using destinations_tuple_t =
            decltype(cib::make_tuple(std::declval<TDestinations>()...));
        constexpr explicit config(TDestinations... dests)
            : logger{cib::make_tuple(dests...)} {}

        log_handler<TCriticalSection, destinations_tuple_t> logger;

        [[noreturn]] static auto terminate() { std::terminate(); }
    };

    // Clang needs a deduction guide here. GCC does not, and in fact GCC has a
    // bug: it claims that deduction guides must be at namespace scope.
#ifdef __clang__
    template <typename... Ts> config(Ts...) -> config<Ts...>;
#endif
};
} // namespace logging::mipi
