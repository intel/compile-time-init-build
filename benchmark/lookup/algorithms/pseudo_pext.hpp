#pragma once

#include <lookup/input.hpp>
#include <lookup/pseudo_pext_lookup.hpp>

#include <array>
#include <cstddef>
#include <cstdio>

#include <nanobench.h>

namespace pp {
template <auto data, typename T>
constexpr auto input_data = []() {
    std::array<lookup::entry<T, T>, data.size()> d{};

    for (auto i = std::size_t{}; i < d.size(); i++) {
        d[i] = {static_cast<T>(data[i].first), static_cast<T>(data[i].second)};
    }

    return d;
}();
} // namespace pp

template <auto data, typename T, bool indirect = true,
          std::size_t max_search_len = 2>
constexpr auto make_pseudo_pext() {
    return lookup::pseudo_pext_lookup<indirect, max_search_len>::make(
        CX_VALUE(lookup::input{0, pp::input_data<data, T>}));
}

template <auto data, typename T, bool indirect = true,
          std::size_t max_search_len = 2>
__attribute__((noinline, flatten)) T do_pseudo_pext(T k) {
    constexpr static auto map =
        make_pseudo_pext<data, T, indirect, max_search_len>();
    return map[k];
}

template <auto data, typename T, bool indirect = true,
          std::size_t max_search_len = 2>
void bench_pseudo_pext(auto name) {
    constexpr static auto map =
        make_pseudo_pext<data, T, indirect, max_search_len>();

    printf("size:      %lu\n", sizeof(map));

    T k = static_cast<T>(data[0].first);

    do_pseudo_pext<data, T, indirect, max_search_len>(k);
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

template <auto data, typename T> void bench_pseudo_pext_direct(auto name) {
    bench_pseudo_pext<data, T, false, 1>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_1(auto name) {
    bench_pseudo_pext<data, T, true, 1>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_2(auto name) {
    bench_pseudo_pext<data, T, true, 2>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_3(auto name) {
    bench_pseudo_pext<data, T, true, 3>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_4(auto name) {
    bench_pseudo_pext<data, T, true, 4>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_5(auto name) {
    bench_pseudo_pext<data, T, true, 5>(name);
}

template <auto data, typename T> void bench_pseudo_pext_indirect_6(auto name) {
    bench_pseudo_pext<data, T, true, 6>(name);
}
