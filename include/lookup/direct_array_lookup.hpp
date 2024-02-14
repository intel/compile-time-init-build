#pragma once
#include <lookup/detail/select.hpp>
#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace lookup {
template <int MinLoadFactor> struct direct_array_lookup {
  private:
    constexpr static auto key_lt = [](auto lhs, auto rhs) {
        return lhs.key_ < rhs.key_;
    };

    constexpr static auto min_key_for(auto const &entries) {
        return std::min_element(entries.begin(), entries.end(), key_lt)->key_;
    }

    constexpr static auto compute_storage_size(auto const &entries,
                                               auto min_key) {
        auto const max_key =
            std::max_element(entries.begin(), entries.end(), key_lt)->key_;
        return max_key - min_key + 2;
    }

    constexpr static auto compute_load_factor(auto const &entries,
                                              auto storage_size) -> int {
        auto const num_keys = static_cast<int>(entries.size());
        return (num_keys * 100) / static_cast<int>(storage_size);
    }

    template <typename T, auto MinKey, std::size_t N> struct impl {
      private:
        using key_type = decltype(MinKey);
        std::array<T, N> storage{};

      public:
        constexpr explicit impl(auto const &input) {
            storage.fill(input.default_value);

            for (auto [k, v] : input.entries) {
                auto const index = k - MinKey;
                storage[index] = v;
            }
        }

        [[nodiscard]] constexpr auto operator[](key_type key) const -> T {
            auto const index = key - MinKey;
            auto const idx = detail::select_lt(
                static_cast<std::uint32_t>(index),
                static_cast<std::uint32_t>(storage.size()),
                static_cast<std::uint32_t>(index),
                static_cast<std::uint32_t>(storage.size()) - 1);
            return storage[idx];
        }
    };

  public:
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto i) {
        constexpr auto input = i();
        using value_type =
            typename decltype(input.entries)::value_type::value_type;
        if constexpr (not std::is_integral_v<value_type> or
                      std::size(input.entries) == 0) {
            return strategy_failed_t{};
        } else {
            constexpr auto min_key = min_key_for(input.entries);
            constexpr auto N = compute_storage_size(input.entries, min_key);
            if constexpr (constexpr auto load_factor =
                              compute_load_factor(input.entries, N);
                          load_factor >= MinLoadFactor) {
                return impl<value_type, min_key, N>(input);
            } else {
                return strategy_failed_t{};
            }
        }
    }
};
} // namespace lookup
