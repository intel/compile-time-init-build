#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/tuple.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace cib::detail {
// https://en.wikipedia.org/wiki/Quicksort#Hoare_partition_scheme

template <typename T>
CIB_CONSTEVAL static auto partition(T *elems, std::size_t lo, std::size_t hi)
    -> std::size_t {
    auto const pivot = elems[(hi + lo) / 2];

    auto i = lo - 1;
    auto j = hi + 1;

    while (true) {
        do {
            i = i + 1;
        } while (elems[i] < pivot);
        do {
            j = j - 1;
        } while (elems[j] > pivot);
        if (i >= j) {
            return j;
        }

        auto const temp = elems[i];
        elems[i] = elems[j];
        elems[j] = temp;
    }
}

template <typename T>
CIB_CONSTEVAL static void quicksort(T *elems, std::size_t lo, std::size_t hi) {
    if (lo < hi) {
        auto const p = partition(elems, lo, hi);
        quicksort(elems, lo, p);
        quicksort(elems, p + 1, hi);
    }
}

template <typename T> CIB_CONSTEVAL static void quicksort(T &collection) {
    quicksort(std::begin(collection), 0,
              std::size(collection) - std::size_t{1});
}
} // namespace cib::detail

namespace cib::detail {
struct typename_map_entry {
    std::string_view name{};
    std::size_t index{};
    std::size_t src{};

  private:
    [[nodiscard]] constexpr friend auto
    operator==(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return lhs.name == rhs.name;
    }

    [[nodiscard]] constexpr friend auto
    operator!=(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return not(lhs == rhs);
    }

    [[nodiscard]] constexpr friend auto operator<(typename_map_entry const &lhs,
                                                  typename_map_entry const &rhs)
        -> bool {
        return lhs.name < rhs.name; // NOLINT(modernize-use-nullptr)
    }

    [[nodiscard]] constexpr friend auto operator>(typename_map_entry const &lhs,
                                                  typename_map_entry const &rhs)
        -> bool {
        return rhs < lhs;
    }

    [[nodiscard]] constexpr friend auto
    operator<=(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return not(rhs < lhs);
    }

    [[nodiscard]] constexpr friend auto
    operator>=(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return not(lhs < rhs);
    }
};

template <typename Tag> CIB_CONSTEVAL static auto name() -> std::string_view {
#if defined(__clang__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto lhs = 44;
    constexpr auto rhs = function_name.size() - 2;

#elif defined(__GNUC__) || defined(__GNUG__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto lhs = 59;
    constexpr auto rhs = function_name.size() - 51;

#elif defined(_MSC_VER)
    constexpr std::string_view function_name = __FUNCSIG__;
    constexpr auto lhs = 0;
    constexpr auto rhs = function_name.size();
#else
    static_assert(false, "Unknown compiler, can't build cib::tuple name.");
#endif

    return function_name.substr(lhs, rhs - lhs + 1);
}

template <typename MetaFunc, typename... Types>
CIB_CONSTEVAL static auto create_type_names([[maybe_unused]] std::size_t src) {
    if constexpr (sizeof...(Types) == 0) {
        return std::array<typename_map_entry, 0>{};

    } else {
        std::size_t i = 0;

        std::array<typename_map_entry, sizeof...(Types)> names = {
            typename_map_entry{
                name<typename MetaFunc::template invoke<Types>>(), i++,
                src}...};

        detail::quicksort(names);
        return names;
    }
}
} // namespace cib::detail
