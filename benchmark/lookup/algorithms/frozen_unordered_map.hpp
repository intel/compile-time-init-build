#pragma once

#include <cstddef>
#include <cstdio>
#include <utility>

#include <frozen/unordered_map.h>
#include <nanobench.h>

template <auto data, typename T> constexpr auto make_frozen_unordered_map() {
    return []<std::size_t... i>(std::index_sequence<i...>) {
        return frozen::unordered_map<T, T, data.size()>{
            {{static_cast<T>(data[i].first),
              static_cast<T>(data[i].second)}...}};
    }(std::make_index_sequence<data.size()>{});
}

template <auto data, typename T>
__attribute__((noinline, flatten)) T do_frozen_unordered_map(T k) {
    constexpr static auto map = make_frozen_unordered_map<data, T>();
    return map.find(k)->second;
}

template <auto data, typename T> void bench_frozen_unordered_map(auto name) {

    constexpr auto map = make_frozen_unordered_map<data, T>();

    printf("size:      %lu\n", sizeof(map));

    do_frozen_unordered_map<data, T>(T{});

    T k = static_cast<T>(data[0].first);
    ankerl::nanobench::Bench().minEpochIterations(2000000).run("chained", [&] {
        k = map.find(k)->second;
        ankerl::nanobench::doNotOptimizeAway(k);
    });

    auto i = std::size_t{};
    ankerl::nanobench::Bench().minEpochIterations(2000000).run(
        "independent", [&] {
            auto v = map.find(data[i].first)->second;
            i++;
            if (i >= data.size()) {
                i = 0;
            }
            ankerl::nanobench::doNotOptimizeAway(v);
        });
}
