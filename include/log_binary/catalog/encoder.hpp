#pragma once

#include <log/log.hpp>
#include <log/module.hpp>
#include <log/unit.hpp>
#include <log_binary/catalog/arguments.hpp>
#include <log_binary/catalog/builder.hpp>
#include <log_binary/catalog/catalog.hpp>
#include <log_binary/catalog/writer.hpp>
#include <log_binary/module_id.hpp>
#include <log_binary/string_id.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <conc/concurrency.hpp>

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

namespace logging::binary {
namespace detail {
template <stdx::ct_string S> CONSTEVAL static auto to_string_type() {
    constexpr auto s = std::string_view{S};
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::string<s[Is]...>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <typename NamedArg> constexpr static auto to_named_arg_type() {
    using str = decltype(to_string_type<NamedArg::name>());
    return sc::named_arg<str, NamedArg::begin, NamedArg::end>{};
}

template <typename S, typename NamedArgs, auto Id, typename... Args>
constexpr static auto to_message() {
    using str = decltype(to_string_type<S::value>());
    return stdx::apply_sequence<NamedArgs>([]<typename... NAs>() {
        return sc::message<sc::undefined<
            sc::args<Args...>, Id, str,
            sc::named_args<decltype(to_named_arg_type<NAs>())...>>>{};
    });
}

template <stdx::ct_string S, auto Id> constexpr static auto to_module() {
    using str = decltype(to_string_type<S>());
    return sc::module_string<sc::undefined<void, Id, str>>{};
}

template <typename S, typename NamedArgs, auto Id> struct to_message_t {
    template <typename... Args>
    using fn = decltype(to_message<S, NamedArgs, Id, Args...>());
};

} // namespace detail

template <typename Destinations> struct log_writer {
    auto operator()(auto msg) -> void {
        stdx::for_each(
            [&]<typename Dest>(Dest &dest) {
                conc::call_in_critical_section<Dest>([&] { dest(msg); });
            },
            dests);
    }

    Destinations dests;
};
template <typename T> log_writer(T) -> log_writer<T>;

template <writer_like Writer> struct log_handler {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename FmtResult>
    auto log(FilenameStringType, LineNumberType, FmtResult const &fr) -> void {
        log_msg<Env>(fr);
    }

    template <typename Env, typename FmtResult>
    auto log_msg(FmtResult const &fr) -> void {
        auto builder = get_builder(Env{});
        writer_like auto writer = stdx::query<Env>(get_writer, w);
        fr.args.apply([&]<typename... Args>(Args &&...args) {
            constexpr auto L = stdx::to_underlying(get_level(Env{}));
            using Message = typename decltype(builder)::template convert_args<
                detail::to_message_t<
                    decltype(fr.str), typename FmtResult::named_args_t,
                    logging::get_string_id(Env{})>::template fn,
                std::remove_cvref_t<Args>...>;
            using Module =
                decltype(detail::to_module<get_module(Env{}),
                                           logging::get_module_id(Env{})>());
            auto const unit = get_unit(Env{})();
            auto const pkt =
                builder.template build<L>(catalog<Message>(), module<Module>(),
                                          unit, std::forward<Args>(args)...);
            writer(pkt.as_const_view().data());
        });
    }

    template <typename Env, auto Version, stdx::ct_string S = "">
    auto log_version() -> void {
        auto builder = get_builder(Env{});
        writer_like auto writer = stdx::query<Env>(get_writer, w);
        auto const pkt = builder.template build_version<Version, S>();
        writer(pkt.as_const_view().data());
    }

    Writer w;
};

template <writer_like... Dests> struct config {
    using destinations_tuple_t = stdx::tuple<Dests...>;
    constexpr explicit config(Dests... dests)
        : logger{log_writer{stdx::tuple{std::move(dests)...}}} {}

    log_handler<log_writer<destinations_tuple_t>> logger;
};

template <typename... Ts> config(Ts...) -> config<Ts...>;
} // namespace logging::binary
