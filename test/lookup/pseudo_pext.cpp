#define ANKERL_NANOBENCH_IMPLEMENT
#include "lookup_datasets.hpp"

#include <lookup/input.hpp>
#include <lookup/pseudo_pext_lookup.hpp>

#include <stdx/utility.hpp>

#include <map>
#include <memory>
#include <mph>
#include <unordered_map>

#include <frozen/map.h>
#include <frozen/unordered_map.h>
#include <nanobench.h>

std::size_t allocated_size = 0;

template <typename T> class TrackingAllocator {
  public:
    using value_type = T;

    TrackingAllocator() noexcept {}

    template <typename U>
    TrackingAllocator(TrackingAllocator<U> const &) noexcept {}

    template <typename U> struct rebind {
        using other = TrackingAllocator<U>;
    };

    T *allocate(std::size_t n) {
        T *ptr = std::allocator<T>{}.allocate(n);
        allocated_size += (n * sizeof(T));
        return ptr;
    }

    void deallocate(T *p, std::size_t n) {
        std::allocator<T>{}.deallocate(p, n);
        allocated_size -= (n * sizeof(T));
    }
};

// Specialize std::allocator_traits for TrackingAllocator
// namespace std {
//     template <typename T>
//     struct allocator_traits<TrackingAllocator<T>> {
//         using allocator_type = TrackingAllocator<T>;
//         using value_type = T;

//         static T* allocate(allocator_type& a, std::size_t n) {
//             return a.allocate(n);
//         }

//         static void deallocate(allocator_type& a, T* p, std::size_t n) {
//             a.deallocate(p, n);
//         }
//     };
// }

enum class key_gen { INDEPENDENT, CHAINED };

template <auto data, typename T, bool indirect = true,
          std::size_t max_search_len = 2>
constexpr auto make_pseudo_pext() {
    constexpr static auto input_data = []() {
        std::array<lookup::entry<T, T>, data.size()> d{};

        for (auto i = std::size_t{}; i < d.size(); i++) {
            d[i] = {static_cast<T>(data[i].first),
                    static_cast<T>(data[i].second)};
        }

        return d;
    }();

    constexpr static auto map =
        lookup::pseudo_pext_lookup<indirect, max_search_len>::make(
            CX_VALUE(lookup::input{0, input_data}));

    return map;
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

template <auto data, typename T> void bench_std_unordered_map(auto name) {
    using allocator_t = TrackingAllocator<std::pair<T const, T>>;
    auto map = std::unordered_map<T, T, std::hash<int>, std::equal_to<int>,
                                  allocator_t>{};

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

template <auto data, typename T> constexpr auto make_mph() {
    constexpr static auto input_data =
        []<std::size_t... i>(std::index_sequence<i...>) {
            return std::array<mph::pair<T, T>, data.size()>{
                {{static_cast<T>(data[i].first),
                  static_cast<T>(data[i].second)}...}};
        }(std::make_index_sequence<data.size()>{});

    constexpr auto map = mph::hash<input_data>;

    return map;
}

template <auto data, typename T>
__attribute__((noinline, flatten)) T do_mph(T k) {
    constexpr static auto map = make_mph<data, T>();
    return *map(k);
}

template <auto data, typename T> void bench_mph(auto name) {

    constexpr auto map = make_mph<data, T>();

    // printf("\nmph\n");
    do_mph<data, T>(T{});

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

template <auto data, typename T> constexpr auto make_frozen_map() {
    return []<std::size_t... i>(std::index_sequence<i...>) {
        return frozen::map<T, T, data.size()>{
            {{static_cast<T>(data[i].first),
              static_cast<T>(data[i].second)}...}};
    }(std::make_index_sequence<data.size()>{});
}

template <auto data, typename T>
__attribute__((noinline, flatten)) T do_frozen_map(T k) {
    constexpr static auto map = make_frozen_map<data, T>();
    return map.find(k)->second;
}

template <auto data, typename T> void bench_frozen_map(auto name) {

    constexpr auto map = make_frozen_map<data, T>();

    printf("size:      %lu\n", sizeof(map));

    do_frozen_map<data, T>(T{});

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

int main() {
    printf("\n\n\ndataset:   %s\n", QBENCH_DATASET);
    printf("algorithm: %s\n", QALG_NAME);
    ALG_NAME<BENCH_DATASET, decltype(BENCH_DATASET[0].first)>(QBENCH_DATASET);
}
