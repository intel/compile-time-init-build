#pragma once

#include "allocator.hpp"

#include <cstddef>
#include <cstdio>
#include <functional>
#include <map>
#include <utility>

#include <nanobench.h>

template <auto data, typename T> void bench_std_map(auto name) {
    using allocator_t = TrackingAllocator<std::pair<T const, T>>;
    auto map = std::map<T, T, std::less<int>, allocator_t>{};

    for (auto p : data) {
        map[p.first] = p.second;
    }

    printf("size:      %lu\n", sizeof(map) + allocated_size);

    T k = static_cast<T>(data[0].first);
    ankerl::nanobench::Bench().minEpochIterations(2000000).run("chained", [&] {
        k = map[k];
        ankerl::nanobench::doNotOptimizeAway(k);
    });

    auto i = std::size_t{};
    ankerl::nanobench::Bench().minEpochIterations(2000000).run(
        "independent", [&] {
            auto v = map[static_cast<T>(data[i].first)];
            i++;
            if (i >= data.size()) {
                i = 0;
            }
            ankerl::nanobench::doNotOptimizeAway(v);
        });
}
