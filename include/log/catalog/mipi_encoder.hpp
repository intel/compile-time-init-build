#pragma once

#include <cib/detail/compiler.hpp>
#include <log/catalog/catalog.hpp>
#include <log/log_level.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

#define CIB_LOG(LEVEL, MSG, ...)                                               \
    mipi::logger_impl::log_impl<LEVEL>(                                        \
        mipi::format_helper{MSG##_sc}.f(__VA_ARGS__))

#define CIB_TRACE(...) CIB_LOG(log_level::TRACE, __VA_ARGS__)
#define CIB_INFO(...) CIB_LOG(log_level::INFO, __VA_ARGS__)
#define CIB_WARN(...) CIB_LOG(log_level::WARN, __VA_ARGS__)
#define CIB_ERROR(...) CIB_LOG(log_level::ERROR, __VA_ARGS__)
#define CIB_FATAL(...) CIB_LOG(log_level::FATAL, __VA_ARGS__)

#define CIB_ASSERT(expr)

namespace mipi {
template <typename T> struct format_helper {
    constexpr static T str{};

    constexpr explicit format_helper(T) {}

    template <typename... Ts>
    CIB_ALWAYS_INLINE constexpr static auto f(Ts... args) {
        return format(str, args...);
    }
};

template <typename CriticalSectionRaii, typename... LogDestinations>
struct mipi_encoder {
  private:
    CIB_CONSTEVAL static auto make_catalog32_header(log_level level)
        -> std::uint32_t {
        return (0x1u << 24u) | // mipi sys-t subtype: id32_p32
               (static_cast<std::uint32_t>(level) << 4u) |
               0x3u; // mipi sys-t type: catalog
    }

    CIB_CONSTEVAL static auto make_short32_header(string_id id)
        -> std::uint32_t {
        return (id << 4u) | 1u;
    }

    template <typename... MsgDataTypes>
    CIB_NEVER_INLINE static void
    dispatch_pass_by_args(MsgDataTypes... msg_data) {
        CriticalSectionRaii cs{};
        (LogDestinations::log_by_args(msg_data...), ...);
    }

    CIB_NEVER_INLINE static void
    dispatch_pass_by_buffer(std::uint32_t *msg, std::uint32_t msg_size) {
        CriticalSectionRaii cs{};
        (LogDestinations::log_by_buf(msg, msg_size), ...);
    }

    template <log_level Level, typename... MsgDataTypes>
    CIB_ALWAYS_INLINE static void dispatch_message(string_id id,
                                                   MsgDataTypes &&...msg_data) {
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

  public:
    template <log_level Level, typename StringType>
    CIB_ALWAYS_INLINE static void log_impl(StringType msg) {
        msg.args.apply([](auto... args) {
            using Message = message<Level, StringType>;
            dispatch_message<Level>(catalog<Message>(),
                                    static_cast<std::uint32_t>(args)...);
        });
    }

    CIB_ALWAYS_INLINE static void log_id(string_id id) {
        dispatch_message<log_level::TRACE>(id);
    }
};

#ifndef CIB_LOG_CRITICAL_SECTION_RAII_TYPE
struct CIB_LOG_CRITICAL_SECTION_RAII_TYPE {};
struct CIB_LOG_DESTINATIONS {
    template <typename... Args> constexpr static auto log_by_args(Args &&...) {}
    template <typename... Args> constexpr static auto log_by_buf(Args &&...) {}
};
#endif

using logger_impl =
    mipi_encoder<CIB_LOG_CRITICAL_SECTION_RAII_TYPE, CIB_LOG_DESTINATIONS>;
} // namespace mipi
