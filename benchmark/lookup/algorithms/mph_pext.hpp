#pragma once

#include <array>
#include <cstddef>
#include <mph>
#include <utility>

#include <nanobench.h>

template <auto data, typename T> constexpr auto make_mph_pext() {
    constexpr static auto input_data =
        []<std::size_t... i>(std::index_sequence<i...>) {
            return std::array<mph::pair<T, T>, data.size()>{
                {{static_cast<T>(data[i].first),
                  static_cast<T>(data[i].second)}...}};
        }(std::make_index_sequence<data.size()>{});

    constexpr auto conditional_pext_policy =
        []<auto const... ts>(auto &&...args) {
            return mph::pext<32u>{}.template operator()<ts...>(
                std::forward<decltype(args)>(args)...);
        };

    constexpr auto map = mph::hash<input_data, conditional_pext_policy>;

    return map;
}

template <auto data, typename T>
__attribute__((noinline, flatten)) T do_mph_pext(T k) {
    constexpr static auto map = make_mph_pext<data, T>();
    return *map(k);
}

template <auto data, typename T> void bench_mph_pext(auto name) {

    constexpr auto map = make_mph_pext<data, T>();

    // printf("\nmph\n");
    do_mph_pext<data, T>(T{});

    T k = static_cast<T>(data[0].first);
    ankerl::nanobench::Bench().minEpochIterations(2000000).run("chained", [&] {
        k = *map(k);
        ankerl::nanobench::doNotOptimizeAway(k);
    });

    auto i = std::size_t{};
    ankerl::nanobench::Bench().minEpochIterations(2000000).run(
        "independent", [&] {
            auto v = *map(static_cast<T>(data[i].first));
            i++;
            if (i >= data.size()) {
                i = 0;
            }
            ankerl::nanobench::doNotOptimizeAway(v);
        });
}
