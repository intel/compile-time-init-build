#pragma once

#include <log/catalog/catalog.hpp>
#include <log/log_level.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <cstdint>
#include <tuple>

#define LOG(LEVEL, MSG, ...)                                                   \
    log::logger_impl::log_impl<LEVEL>(                                         \
        log::format_helper{MSG##_sc}.f(__VA_ARGS__))

#define TRACE(...) LOG(log_level::TRACE, __VA_ARGS__)
#define INFO(...) LOG(log_level::INFO, __VA_ARGS__)
#define WARN(...) LOG(log_level::WARN, __VA_ARGS__)
#define ERROR(...) LOG(log_level::ERROR, __VA_ARGS__)
#define FATAL(...) LOG(log_level::FATAL, __VA_ARGS__)

#define ASSERT(expr)

#ifndef CIB_LOG_ALWAYS_INLINE
#define CIB_LOG_ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#ifndef CIB_LOG_NEVER_INLINE
#define CIB_LOG_NEVER_INLINE __attribute__((noinline))
#endif

namespace log {
template <typename T> struct format_helper {
    constexpr static T str{};

    constexpr explicit format_helper(T) {}

    template <typename... Ts>
    CIB_LOG_ALWAYS_INLINE constexpr static auto f(Ts... args) {
        return format(str, args...);
    }
};

template <typename CriticalSectionRaii, typename... LogDestinations>
struct mipi_encoder {
  private:
    constexpr static auto MAX_ARG_LENGTH = sc::int_<3>;

    template <typename... MsgDataTypes>
    CIB_LOG_NEVER_INLINE static void
    dispatch_pass_by_args(MsgDataTypes... msg_data) {
        CriticalSectionRaii cs;
        (LogDestinations::log_by_args(msg_data...), ...);
    }

    template <log_level level, typename IdType, typename... MsgDataTypes>
    CIB_LOG_NEVER_INLINE static void
    log_pass_by_args(IdType id, MsgDataTypes... msg_data) {
        if constexpr (sizeof...(msg_data) == 0) {
            dispatch_pass_by_args((id << 4) | 1);
        } else {
            uint32_t constexpr normal_header_dw =
                (0x1 << 24) |       // mipi sys-t subtype: id32_p32
                (level << 4) | 0x3; // mipi sys-t type: catalog

            dispatch_pass_by_args(normal_header_dw, id, msg_data...);
        }
    }

    CIB_LOG_NEVER_INLINE static void
    dispatch_pass_by_buffer(std::uint32_t *msg, std::uint32_t msg_size) {
        CriticalSectionRaii cs;
        (LogDestinations::log_by_buf(msg, msg_size), ...);
    }

    template <log_level level>
    CIB_LOG_NEVER_INLINE static void
    log_pass_by_buffer(std::uint32_t *msg, std::uint32_t msg_size) {
        msg[0] = (0x1 << 24) |       // mipi sys-t subtype: id32_p32
                 (level << 4) | 0x3; // mipi sys-t type: catalog

        dispatch_pass_by_buffer(msg, msg_size);
    }

    /**
     * @tparam MsgDataTupleType
     * @param msg_data_tuple
     */
    template <log_level level, typename MsgDataTupleType>
    CIB_LOG_ALWAYS_INLINE static void
    dispatch_message(MsgDataTupleType msg_data_tuple) {
        constexpr auto msg_size = std::tuple_size_v<MsgDataTupleType>;

        if constexpr (msg_size <= MAX_ARG_LENGTH) {
            std::apply(
                [&](auto... payload) { log_pass_by_args<level>(payload...); },
                msg_data_tuple);
        } else {
            // we need an additional dword for mipi syst header
            auto const full_msg_size = msg_size + 1;

            std::apply(
                [&](auto... payload) {
                    std::uint32_t full_args_array[full_msg_size] = {0,
                                                                    payload...};
                    log_pass_by_buffer<level>(full_args_array, full_msg_size);
                },
                msg_data_tuple);
        }
    }

    template <typename IdType, typename ArgTupleType>
    CIB_LOG_ALWAYS_INLINE static auto to_tuple(IdType id,
                                               ArgTupleType arg_tuple) {
        auto const normal_headers = std::make_tuple(id);

        auto const normal_payloads = std::apply(
            [&](auto... args_pack) {
                return std::make_tuple(
                    static_cast<std::uint32_t>(args_pack)...);
            },
            arg_tuple);

        return std::tuple_cat(normal_headers, normal_payloads);
    }

  public:
    template <log_level level, typename StringType>
    CIB_LOG_ALWAYS_INLINE static void log_impl(StringType msg) {
        using Message = message<level, StringType>;
        auto const id = catalog<Message>();
        auto const msg_tuple = to_tuple(id, msg.args);
        dispatch_message<level>(msg_tuple);
    }

    CIB_LOG_ALWAYS_INLINE static void log_id(string_id id) {
        auto const msg_tuple = to_tuple(id, std::make_tuple());
        dispatch_message<log_level::TRACE>(msg_tuple);
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
} // namespace log
