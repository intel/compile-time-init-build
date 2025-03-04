#pragma once

#include <conc/concurrency.hpp>
#include <log/catalog/catalog.hpp>
#include <log/catalog/mipi_builder.hpp>
#include <log/catalog/mipi_messages.hpp>
#include <log/log.hpp>
#include <log/module.hpp>

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
template <typename S, typename... Args> constexpr auto to_message() {
    constexpr auto s = S::value;
    using char_t = typename std::remove_cv_t<decltype(s)>::value_type;
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::message<
            sc::undefined<sc::args<encode_as_t<Args>...>, char_t, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <stdx::ct_string S> constexpr auto to_module() {
    constexpr auto s = std::string_view{S};
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::module_string<sc::undefined<void, char, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}
} // namespace detail

template <typename TDestinations> struct log_handler {
    constexpr explicit log_handler(TDestinations &&ds) : dests{std::move(ds)} {}

    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    auto log(FilenameStringType, LineNumberType, MsgType const &msg) -> void {
        log_msg<Env>(msg);
    }

    template <typename Env, typename Msg> auto log_msg(Msg msg) -> void {
        msg.apply([&]<typename S, typename... Args>(S, Args... args) {
            constexpr auto L = stdx::to_underlying(get_level(Env{}).value);
            using Message = decltype(detail::to_message<S, Args...>());
            using Module =
                decltype(detail::to_module<get_module(Env{}).value>());
            auto builder = get_builder(Env{}).value;
            write(builder.template build<L>(catalog<Message>(),
                                            module<Module>(), args...));
        });
    }

    template <auto Version, stdx::ct_string S = ""> auto log_build() -> void {
        using namespace msg;
        if constexpr (S.empty() and stdx::bit_width(Version) <= 22) {
            owning<defn::compact32_build_msg_t> message{"build_id"_field =
                                                            Version};
            write(message);
        } else if constexpr (S.empty() and stdx::bit_width(Version) <= 54) {
            owning<defn::compact64_build_msg_t> message{"build_id"_field =
                                                            Version};
            write(message);
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
            write(message);
        }
    }

  private:
    template <std::size_t N>
    auto write(stdx::span<std::uint8_t const, N> msg) -> void {
        stdx::for_each(
            [&]<typename Dest>(Dest &dest) {
                conc::call_in_critical_section<Dest>(
                    [&] { dest.log_by_buf(msg); });
            },
            dests);
    }

    template <std::size_t N>
    auto write(stdx::span<std::uint32_t const, N> msg) -> void {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            stdx::for_each(
                [&]<typename Dest>(Dest &dest) {
                    conc::call_in_critical_section<Dest>(
                        [&] { dest.log_by_args(msg[Is]...); });
                },
                dests);
        }(std::make_index_sequence<N>{});
    }

    auto write(auto const &msg) -> void { write(msg.as_const_view().data()); }

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
