#pragma once

#include <lookup/detail/select.hpp>
#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/bit.hpp>
#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>

namespace lookup {

namespace detail {
constexpr auto as_raw_integral(auto v) {
    static_assert(sizeof(v) <= 8);

    if constexpr (sizeof(v) == 1) {
        return std::bit_cast<std::uint8_t>(v);

    } else if constexpr (sizeof(v) == 2) {
        return std::bit_cast<std::uint16_t>(v);

    } else if constexpr (sizeof(v) <= 4) {
        return std::bit_cast<std::uint32_t>(v);

    } else if constexpr (sizeof(v) <= 8) {
        return std::bit_cast<std::uint64_t>(v);
    }
}

template <typename T>
using raw_integral_t = decltype(as_raw_integral(std::declval<T>()));

template <typename T>
constexpr auto compute_pack_coefficient(std::size_t dst, T const mask) -> T {
    constexpr auto t_digits = std::numeric_limits<T>::digits;

    auto pack_coefficient = stdx::bitset<t_digits>{};
    auto const mask_bits = stdx::bitset<t_digits>{mask};

    bool prev_src_bit_set = false;
    for (auto src = std::size_t{}; src < t_digits; src++) {
        bool const curr_src_bit_set = mask_bits[src];
        bool const new_stretch = curr_src_bit_set and not prev_src_bit_set;

        if (new_stretch) {
            pack_coefficient.set(dst - src);
        }

        if (curr_src_bit_set) {
            dst += 1;
        }

        prev_src_bit_set = curr_src_bit_set;
    }

    return pack_coefficient.template to<T>();
}

template <typename T> struct pseudo_pext_t {
    T mask;
    T coefficient;
    T final_mask;
    std::size_t gap_bits;

    constexpr explicit pseudo_pext_t(T mask_arg) : mask{mask_arg} {
        constexpr auto t_digits = std::numeric_limits<T>::digits;
        auto const num_bits_to_extract = std::popcount(mask);
        auto const left_padding = std::countl_zero(mask);
        gap_bits = static_cast<std::size_t>(t_digits - num_bits_to_extract -
                                            left_padding);
        coefficient = compute_pack_coefficient<T>(gap_bits, mask);
        auto const final_mask_msb =
            static_cast<std::size_t>(num_bits_to_extract - 1);
        final_mask = stdx::bit_mask<T>(final_mask_msb);
    }

    [[nodiscard]] constexpr auto operator()(T value) const -> T {
        auto const packed = (value & mask) * coefficient;
        return static_cast<T>(packed >> gap_bits) & final_mask;
    }
};

template <typename T, std::size_t S>
CONSTEVAL auto keys_are_unique(std::array<T, S> const &keys) -> bool {
    for (auto i = std::size_t{}; i < S; i++) {
        for (auto j = i + 1; j < S; j++) {
            if (keys[i] == keys[j]) {
                return false;
            }
        }
    }

    return true;
}

template <typename T, std::size_t S>
CONSTEVAL auto with_mask(T const mask,
                         std::array<T, S> const &keys) -> std::array<T, S> {
    std::array<T, S> new_keys{};

    std::transform(keys.begin(), keys.end(), new_keys.begin(),
                   [&](T k) { return pseudo_pext_t(mask)(k); });

    return new_keys;
}

template <typename T, typename V, std::size_t S>
CONSTEVAL auto get_keys(std::array<entry<T, V>, S> const &entries) {
    using raw_t = detail::raw_integral_t<T>;
    std::array<raw_t, S> new_keys{};

    std::transform(
        entries.begin(), entries.end(), new_keys.begin(),
        [&](entry<T, V> e) { return detail::as_raw_integral(e.key_); });

    return new_keys;
}

template <typename T, typename V, std::size_t S>
CONSTEVAL auto calc_pseudo_pext_mask(std::array<entry<T, V>, S> const &pairs)
    -> detail::raw_integral_t<T> {
    using raw_t = detail::raw_integral_t<T>;

    auto const t_digits = std::numeric_limits<raw_t>::digits;
    std::array<raw_t, S> keys = get_keys(pairs);

    raw_t mask = std::numeric_limits<raw_t>::max();
    for (auto i = std::size_t{}; i < t_digits; i++) {
        raw_t const try_mask = mask & ~static_cast<raw_t>(raw_t{1} << i);
        std::array<raw_t, S> const try_keys = with_mask(try_mask, keys);
        if (keys_are_unique(try_keys)) {
            mask = try_mask;
        }
    }
    return mask;
}
} // namespace detail

struct pseudo_pext_lookup {
  private:
    template <typename Key, typename Value, typename PextFunc, typename Storage>
    struct impl {
        using key_type = Key;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = Value;

        PextFunc pext_func;
        Value default_value;
        Storage storage;

        [[nodiscard]] constexpr auto
        operator[](key_type key) const -> value_type {
            auto const raw_key = detail::as_raw_integral(key);
            auto const e = storage[pext_func(raw_key)];
            return detail::select(raw_key, e.key_, e.value_, default_value);
        }
    };

  public:
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto i) {
        constexpr auto input = i();
        using key_type = typename decltype(input)::key_type;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = typename decltype(input)::value_type;

        constexpr raw_key_type mask =
            detail::calc_pseudo_pext_mask(input.entries);
        constexpr auto p = detail::pseudo_pext_t(mask);
        constexpr auto storage_size = 1 << std::popcount(mask);
        constexpr auto storage = [&]() {
            std::array<entry<raw_key_type, value_type>, storage_size> s{};

            s.fill({raw_key_type{}, input.default_value});

            for (auto e : input.entries) {
                raw_key_type const k = detail::as_raw_integral(e.key_);
                s[p(k)] = {k, e.value_};
            }

            return s;
        }();

        return impl<key_type, value_type, decltype(p), decltype(storage)>{
            p, input.default_value, storage};
    }
};
} // namespace lookup
