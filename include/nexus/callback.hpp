#pragma once

#include <stdx/compiler.hpp>
#include <stdx/panic.hpp>
#include <stdx/tuple.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace callback {
template <typename Funcs = stdx::tuple<>, typename... ArgTypes> struct builder {
    constexpr static auto size =
        std::integral_constant<std::size_t, stdx::tuple_size_v<Funcs>>{};

    Funcs funcs{};

    template <typename... Fs>
    [[nodiscard]] constexpr auto add(Fs &&...fs) const {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            using T = stdx::tuple<
                std::remove_cvref_t<decltype(funcs[stdx::index<Is>])>...,
                std::decay_t<std::remove_cvref_t<Fs>>...>;
            return builder<T, ArgTypes...>{
                .funcs = {funcs[stdx::index<Is>]..., std::forward<Fs>(fs)...}};
        }(std::make_index_sequence<size()>{});
    }

    template <typename BuilderValue, typename... Ts>
    [[nodiscard]] CONSTEVAL static auto build() {
        return run<BuilderValue, Ts...>;
    }

  private:
    template <typename BuilderValue, typename... Ts>
    constexpr static void run(ArgTypes... args) {
        constexpr auto b = BuilderValue::value;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (invoke_with<Ts...>(b.funcs[stdx::index<Is>], args...), ...);
        }(std::make_index_sequence<b.size()>{});
    }

    template <typename... Ts>
    constexpr static auto invoke_with =
        []<typename F, typename... Args>(F &&f, Args &&...args) -> void {
        if constexpr (requires {
                          std::forward<F>(f).template operator()<Ts...>(
                              std::forward<Args>(args)...);
                      }) {
            std::forward<F>(f).template operator()<Ts...>(
                std::forward<Args>(args)...);
        } else {
            std::forward<F>(f)(std::forward<Args>(args)...);
        }
    };
};

template <typename... ArgTypes> struct service {
    using builder_t = builder<stdx::tuple<>, ArgTypes...>;
    using interface_t = void (*)(ArgTypes...);

    CONSTEVAL static auto uninitialized() -> interface_t {
        return [](ArgTypes...) {
            stdx::panic<
                "Attempting to run callback before it is initialized">();
        };
    }
};
} // namespace callback
