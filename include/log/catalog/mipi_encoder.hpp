#pragma once

#include <conc/concurrency.hpp>
#include <log/catalog/catalog.hpp>
#include <log/log.hpp>
#include <log/module.hpp>
#include <msg/message.hpp>

#include <stdx/bit.hpp>
#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <utility>

namespace logging::mipi {
namespace detail {
template <logging::level L, typename S, typename... Args>
constexpr auto to_message() {
    constexpr auto s = S::value;
    using char_t = typename std::remove_cv_t<decltype(s)>::value_type;
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::message<
            L, sc::undefined<sc::args<Args...>, char_t, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <stdx::ct_string S> constexpr auto to_module() {
    constexpr auto s = std::string_view{S};
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::module_string<sc::undefined<void, char, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}
} // namespace detail

namespace defn {
using msg::at;
using msg::dword_index_t;
using msg::field;
using msg::message;
using msg::operator""_msb;
using msg::operator""_lsb;

enum struct type : uint8_t { Build = 0, Short32 = 1, Catalog = 3 };
enum struct build_subtype : uint8_t { Compact32 = 0, Compact64 = 1, Long = 2 };
enum struct catalog_subtype : uint8_t { Id32_Pack32 = 1 };

using type_f = field<"type", type>::located<at{dword_index_t{0}, 3_msb, 0_lsb}>;
using opt_len_f =
    field<"opt_len", bool>::located<at{dword_index_t{0}, 9_msb, 9_lsb}>;
using payload_len_f =
    field<"payload_len",
          std::uint16_t>::located<at{dword_index_t{1}, 15_msb, 0_lsb}>;

using build_subtype_f =
    field<"subtype",
          build_subtype>::located<at{dword_index_t{0}, 29_msb, 24_lsb}>;
using compact32_build_id_f = field<"build_id", std::uint32_t>::located<
    at{dword_index_t{0}, 31_msb, 30_lsb}, at{dword_index_t{0}, 23_msb, 4_lsb}>;
using compact64_build_id_f = field<"build_id", std::uint64_t>::located<
    at{dword_index_t{1}, 31_msb, 0_lsb}, at{dword_index_t{0}, 31_msb, 30_lsb},
    at{dword_index_t{0}, 23_msb, 4_lsb}>;

using normal_build_msg_t =
    message<"normal_build", type_f::with_required<type::Build>,
            opt_len_f::with_required<true>,
            build_subtype_f::with_required<build_subtype::Long>, payload_len_f>;
using compact32_build_msg_t =
    message<"compact32_build", type_f::with_required<type::Build>,
            build_subtype_f::with_required<build_subtype::Compact32>,
            compact32_build_id_f>;
using compact64_build_msg_t =
    message<"compact64_build", type_f::with_required<type::Build>,
            build_subtype_f::with_required<build_subtype::Compact64>,
            compact64_build_id_f>;

using short32_payload_f =
    field<"payload",
          std::uint32_t>::located<at{dword_index_t{0}, 31_msb, 4_lsb}>;
using short32_msg_t =
    message<"short32", type_f::with_required<type::Short32>, short32_payload_f>;

using catalog_subtype_f =
    field<"subtype",
          catalog_subtype>::located<at{dword_index_t{0}, 29_msb, 24_lsb}>;
using severity_f = field<"severity", std::uint8_t>::located<at{dword_index_t{0},
                                                               6_msb, 4_lsb}>;
using module_id_f =
    field<"module_id",
          std::uint8_t>::located<at{dword_index_t{0}, 22_msb, 16_lsb}>;

using catalog_msg_t =
    message<"catalog", type_f::with_required<type::Catalog>, severity_f,
            module_id_f,
            catalog_subtype_f::with_required<catalog_subtype::Id32_Pack32>>;
} // namespace defn

template <typename TDestinations> struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <logging::level Level, typename Env, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    ALWAYS_INLINE auto log(FilenameStringType, LineNumberType,
                           MsgType const &msg) -> void {
        log_msg<Level, Env>(msg);
    }

    template <logging::level Level, typename Env, typename Msg>
    ALWAYS_INLINE auto log_msg(Msg msg) -> void {
        msg.apply([&]<typename S, typename... Args>(S, Args... args) {
            using Message = decltype(detail::to_message<Level, S, Args...>());
            using Module =
                decltype(detail::to_module<get_module(Env{}).value>());
            dispatch_message<Level>(catalog<Message>(), module<Module>(),
                                    static_cast<std::uint32_t>(args)...);
        });
    }

    template <auto Version, stdx::ct_string S = ""> auto log_build() -> void {
        using namespace msg;
        if constexpr (S.empty() and stdx::bit_width(Version) <= 22) {
            owning<defn::compact32_build_msg_t> message{"build_id"_field =
                                                            Version};
            dispatch_pass_by_args(message.data()[0]);
        } else if constexpr (S.empty() and stdx::bit_width(Version) <= 54) {
            owning<defn::compact64_build_msg_t> message{"build_id"_field =
                                                            Version};
            dispatch_pass_by_args(message.data()[0], message.data()[1]);
        } else {
            constexpr auto header_size =
                defn::normal_build_msg_t::size<std::uint8_t>::value;
            constexpr auto payload_len = S.size() + sizeof(std::uint64_t);
            using storage_t =
                std::array<std::uint8_t, header_size + payload_len>;

            defn::normal_build_msg_t::owner_t<storage_t> message{
                "payload_len"_field = payload_len};
            auto dest = &message.data()[header_size];

            auto const ver = stdx::to_le(static_cast<std::uint64_t>(Version));
            dest = std::copy_n(stdx::bit_cast<std::uint8_t const *>(&ver),
                               sizeof(std::uint64_t), dest);
            std::copy_n(std::cbegin(S.value), S.size(), dest);
            dispatch_pass_by_buffer(message.data());
        }
    }

  private:
    template <typename... MsgDataTypes>
    NEVER_INLINE auto
    dispatch_pass_by_args(MsgDataTypes &&...msg_data) -> void {
        stdx::for_each(
            [&]<typename Dest>(Dest &dest) {
                conc::call_in_critical_section<Dest>([&] {
                    dest.log_by_args(std::forward<MsgDataTypes>(msg_data)...);
                });
            },
            dests);
    }

    NEVER_INLINE auto
    dispatch_pass_by_buffer(stdx::span<std::uint8_t const> msg) -> void {
        stdx::for_each(
            [&]<typename Dest>(Dest &dest) {
                conc::call_in_critical_section<Dest>(
                    [&] { dest.log_by_buf(msg); });
            },
            dests);
    }

    template <logging::level Level, std::same_as<std::uint32_t>... MsgDataTypes>
    ALWAYS_INLINE auto dispatch_message(string_id id,
                                        [[maybe_unused]] module_id m,
                                        MsgDataTypes... msg_data) -> void {
        using namespace msg;
        if constexpr (sizeof...(msg_data) == 0u) {
            owning<defn::short32_msg_t> message{"payload"_field = id};
            dispatch_pass_by_args(message.data()[0]);
        } else if constexpr (sizeof...(MsgDataTypes) <= 2u) {
            owning<defn::catalog_msg_t> message{"severity"_field = Level,
                                                "module_id"_field = m};
            dispatch_pass_by_args(
                message.data()[0], stdx::to_le(id),
                stdx::to_le(std::forward<MsgDataTypes>(msg_data))...);
        } else {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint8_t>::value;
            constexpr auto payload_len =
                (sizeof(id) + ... + sizeof(MsgDataTypes));
            using storage_t =
                std::array<std::uint8_t, header_size + payload_len>;

            defn::catalog_msg_t::owner_t<storage_t> message{
                "severity"_field = Level, "module_id"_field = m};

            constexpr auto copy_arg = [](std::uint32_t arg, auto &dest) {
                std::memcpy(dest, &arg, sizeof(std::uint32_t));
                dest += sizeof(std::uint32_t);
            };
            auto dest = &message.data()[header_size];
            copy_arg(stdx::to_le(id), dest);
            (copy_arg(stdx::to_le(msg_data), dest), ...);
            dispatch_pass_by_buffer(message.data());
        }
    }

    TDestinations dests;
};

template <typename... TDestinations> struct config {
    using destinations_tuple_t = stdx::tuple<TDestinations...>;
    constexpr explicit config(TDestinations... dests)
        : logger{stdx::tuple{std::move(dests)...}} {}

    log_handler<destinations_tuple_t> logger;
};
template <typename... Ts> config(Ts...) -> config<Ts...>;
} // namespace logging::mipi
