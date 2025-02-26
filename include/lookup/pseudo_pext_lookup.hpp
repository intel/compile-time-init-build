#pragma once

#include <lookup/detail/select.hpp>
#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/bit.hpp>
#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <tuple>

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

template <uint64_t BiggestValue> auto uint_for_f() {
    if constexpr (BiggestValue <= std::numeric_limits<uint8_t>::max()) {
        return uint8_t{};

    } else if constexpr (BiggestValue <= std::numeric_limits<uint16_t>::max()) {
        return uint16_t{};

    } else if constexpr (BiggestValue <= std::numeric_limits<uint32_t>::max()) {
        return uint32_t{};

    } else {
        return uint64_t{};
    }
}

template <uint64_t BiggestValue>
using uint_for_ = decltype(uint_for_f<BiggestValue>());

/// log n
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

/// count the number of key duplicates (n log n)
template <typename T, std::size_t S>
constexpr auto count_duplicates(std::array<T, S> keys) -> std::size_t {
    std::sort(std::begin(keys), std::end(keys));
    auto dups = std::size_t{};
    for (auto i = std::adjacent_find(std::cbegin(keys), std::cend(keys));
         i != std::cend(keys); i = std::adjacent_find(++i, std::cend(keys))) {
        ++dups;
    }
    return dups;
}

/// count the length of the longest run of identical values (n)
template <typename T, std::size_t S>
constexpr auto count_longest_run(std::array<T, S> keys) -> std::size_t {
    std::sort(keys.begin(), keys.end());

    auto longest_run = std::size_t{};
    auto current_run = std::size_t{};

    if (S > 0) {
        T prev_value = keys[0];

        for (auto i = std::size_t{1}; i < S; i++) {
            T const curr_value = keys[i];

            if (curr_value == prev_value) {
                current_run++;
            }

            longest_run = std::max(longest_run, current_run);

            if (curr_value != prev_value) {
                current_run = 0;
            }

            prev_value = curr_value;
        }
    }

    return longest_run;
}

template <typename T, std::size_t S>
constexpr auto keys_are_unique(std::array<T, S> const &keys) -> bool {
    return count_duplicates(keys) == 0;
}

template <typename T, std::size_t S>
constexpr auto with_mask(T const mask, std::array<T, S> const &keys)
    -> std::array<T, S> {
    std::array<T, S> new_keys{};

    std::transform(keys.begin(), keys.end(), new_keys.begin(),
                   [&](T k) { return pseudo_pext_t(mask)(k); });

    return new_keys;
}

template <typename T, typename V, std::size_t S>
constexpr auto get_keys(std::array<entry<T, V>, S> const &entries) {
    using raw_t = detail::raw_integral_t<T>;
    std::array<raw_t, S> new_keys{};

    std::transform(
        entries.begin(), entries.end(), new_keys.begin(),
        [&](entry<T, V> e) { return detail::as_raw_integral(e.key_); });

    return new_keys;
}

template <typename T, std::size_t S>
constexpr auto remove_cheapest_bit(detail::raw_integral_t<T> mask,
                                   std::array<T, S> keys)
    -> detail::raw_integral_t<T> {
    using raw_t = detail::raw_integral_t<T>;

    auto const t_digits = std::numeric_limits<raw_t>::digits;
    auto bmask = stdx::bitset<t_digits>{mask};

    auto cheapest_bit = std::size_t{};
    auto min_num_dups = std::numeric_limits<std::size_t>::max();

    for_each(
        [&](auto i) {
            auto btry_mask = bmask;
            btry_mask.reset(i);

            std::array<raw_t, S> try_keys =
                with_mask(btry_mask.template to<raw_t>(), keys);

            auto num_dups = count_duplicates(try_keys);
            if (num_dups < min_num_dups) {
                min_num_dups = num_dups;
                cheapest_bit = i;
            }
        },
        bmask);

    bmask.reset(cheapest_bit);
    return bmask.template to<raw_t>();
}

template <typename T, typename V, std::size_t S>
constexpr auto calc_pseudo_pext_mask(std::array<entry<T, V>, S> const &pairs,
                                     std::size_t max_search_len) {
    using raw_t = detail::raw_integral_t<T>;

    auto const t_digits = std::numeric_limits<raw_t>::digits;
    std::array<raw_t, S> keys = get_keys(pairs);

    // try removing each bit from the mask one at a time.
    // then apply the pseudo_pext function to all the keys with the mask. if
    // the keys are all still unique, then we can remove the bit and move on
    // to the next one.
    raw_t mask = std::numeric_limits<raw_t>::max();
    for (auto x = std::size_t{}; x < t_digits; x++) {
        auto i = t_digits - 1 - x;
        raw_t const try_mask = mask & ~static_cast<raw_t>(raw_t{1} << i);
        std::array<raw_t, S> const try_keys = with_mask(try_mask, keys);
        if (keys_are_unique(try_keys)) {
            mask = try_mask;
        }
    }

    // we can remove more bits from the mask to achieve a smaller memory
    // footprint with a small runtime cost. each additional bit removed
    // from the mask cuts intermediate table size in half, but risks more
    // collisions. try to remove the most number of bits from the mask while
    // staying under the max search length.
    auto prev_longest_run = std::size_t{};
    while (max_search_len > 1 && std::popcount(mask) > 4) {
        auto try_mask = remove_cheapest_bit(mask, keys);
        auto current_longest_run = count_longest_run(with_mask(try_mask, keys));
        if (current_longest_run <= max_search_len) {
            mask = try_mask;
            prev_longest_run = current_longest_run;
        } else {
            return std::make_tuple(mask, prev_longest_run);
        }
    }

    return std::make_tuple(mask, prev_longest_run);
}

} // namespace detail

template <bool Indirect = false, std::size_t MaxSearchLen = 1>
struct pseudo_pext_lookup {
  private:
    constexpr static bool use_indirect_strategy = Indirect;
    static_assert(Indirect or (not Indirect and MaxSearchLen == 1));

    template <typename Key, typename Value, typename Default>
    struct empty_impl {
        using key_type = Key;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = Value;

        constexpr static Value default_value = Default::value;

        [[nodiscard]] constexpr auto operator[](key_type) const -> value_type {
            return default_value;
        }
    };

    template <typename Key, typename Value, typename Default, typename PextFunc,
              typename Storage>
    struct direct_impl {
        using key_type = Key;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = Value;

        PextFunc pext_func;
        constexpr static Value default_value = Default::value;
        Storage storage;

        [[nodiscard]] constexpr auto operator[](key_type key) const
            -> value_type {
            auto const raw_key = detail::as_raw_integral(key);
            auto const e = storage[pext_func(raw_key)];

            if (raw_key == e.key_) {
                return e.value_;
            }

            return default_value;
        }
    };

    // this is a workaround...
    // ...can't have "constexpr static" vars in a constexpr function in c++20
    // ...can't have floats/doubles as nttps
    // so we can use this to smuggle the lambda to the final impl
    template <typename lambda> struct default_value_smuggler {
        constexpr static auto value = lambda{}().default_value;
    };

    template <auto v> struct smuggler {
        constexpr static auto value = v;
    };

    template <typename Key, typename Value, typename Default,
              typename SearchLen, typename PextFunc, typename LookupTable,
              typename Storage>
    struct indirect_impl {
        using key_type = Key;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = Value;

        PextFunc pext_func;
        constexpr static Value default_value = Default::value;
        constexpr static auto search_len = SearchLen::value;
        LookupTable lookup_table;
        Storage storage;

        [[nodiscard]] constexpr auto operator[](key_type key) const
            -> value_type {
            auto const raw_key = detail::as_raw_integral(key);
            auto i = lookup_table[pext_func(raw_key)];

            for (auto search_count = std::size_t{}; search_count < search_len;
                 search_count++) {
                auto const e = storage[i];
                if (raw_key == detail::as_raw_integral(e.key_)) {
                    return e.value_;
                }

                i++;
            }

            return default_value;
        }
    };

  public:
    [[nodiscard]] constexpr static auto make(compile_time auto i) {
        constexpr auto input = i();
        using key_type = typename decltype(input)::key_type;
        using raw_key_type = detail::raw_integral_t<key_type>;
        using value_type = typename decltype(input)::value_type;

        constexpr auto keys = detail::get_keys(input.entries);
        static_assert(detail::keys_are_unique(keys),
                      "Lookup keys must be unique.");

        constexpr auto mask_and_search =
            detail::calc_pseudo_pext_mask(input.entries, MaxSearchLen);

        constexpr auto mask = std::get<0>(mask_and_search);
        constexpr auto search_len = std::get<1>(mask_and_search) + 1;

        using search_len_t = smuggler<search_len>;

        constexpr auto p = detail::pseudo_pext_t(mask);
        constexpr auto lookup_table_size = 1 << std::popcount(mask);

        using default_value = default_value_smuggler<decltype(i)>;

        if constexpr (input.entries.empty()) {
            return empty_impl<key_type, value_type, default_value>{};

        } else if constexpr (use_indirect_strategy) {
            constexpr auto storage = [&]() {
                auto s = input.entries;

                // sort by the hashed key to group all the buckets together
                std::sort(s.begin(), s.end(), [&](auto left, auto right) {
                    return p(detail::as_raw_integral(left.key_)) <
                           p(detail::as_raw_integral(right.key_));
                });

                // find end of the longest bucket
                auto const end_of_longest_bucket = [&]() {
                    auto e = s.begin();

                    auto curr_bucket_length = 1u;
                    auto prev_idx = p(detail::as_raw_integral(e->key_));
                    e++;
                    while (e != s.end()) {
                        auto const curr_idx =
                            p(detail::as_raw_integral(e->key_));

                        if (curr_idx == prev_idx) {
                            curr_bucket_length++;

                        } else if (curr_bucket_length >= search_len) {
                            return e;

                        } else {
                            curr_bucket_length = 1;
                        }

                        prev_idx = curr_idx;
                        e++;
                    }

                    return e;
                }();

                // place the longest bucket at the end
                std::rotate(s.begin(), end_of_longest_bucket, s.end());

                return s;
            }();

            constexpr auto lookup_table = [&]() {
                using lookup_idx_t = detail::uint_for_<storage.size()>;
                std::array<lookup_idx_t, lookup_table_size> t{};

                t.fill(0);

                // iterate backwards so the index of the first entry of a bucket
                // remains in the lookup table
                for (auto entry_idx =
                         static_cast<lookup_idx_t>(storage.size() - 1);
                     entry_idx < storage.size(); entry_idx--) {
                    auto const e = storage[entry_idx];
                    auto const raw_key = detail::as_raw_integral(e.key_);
                    auto const lookup_table_idx = p(raw_key);
                    t[lookup_table_idx] = entry_idx;
                }

                return t;
            }();

            return indirect_impl<key_type, value_type, default_value,
                                 search_len_t, decltype(p),
                                 decltype(lookup_table), decltype(storage)>{
                p, lookup_table, storage};

        } else {
            constexpr auto storage = [&]() {
                std::array<entry<raw_key_type, value_type>, lookup_table_size>
                    s{};

                s.fill({raw_key_type{}, input.default_value});

                for (auto e : input.entries) {
                    raw_key_type const k = detail::as_raw_integral(e.key_);
                    s[p(k)] = {k, e.value_};
                }

                return s;
            }();

            return direct_impl<key_type, value_type, default_value, decltype(p),
                               decltype(storage)>{p, storage};
        }
    }
};
} // namespace lookup

// struct always_t {
//     auto operator()(auto const &) const -> bool {
//         return true;
//     }
// } always{};

// struct never_t {
//     auto operator()(auto const &) const -> bool {
//         return false;
//     }
// } never{};

// template <matcher M>
// auto simplify(M const & m) -> M {
//     return m;
// }

// template <matcher L, matcher R>
// auto simplify(and_t<L, R> const & m) {
//     auto l = simplify(m.lhs);
//     auto r = simplify(m.rhs);
//     if constexpr (/* l is a never_t or r is a never_t */) {
//         return never;
//     } else if constexpr (/* r is an always_t */) {
//         return l;
//     } else if constexpr (/* l is an always_t */) {
//         return r;
//     } else {
//         return and_t{l, r};
//     }
// }

// template <matcher M>
// auto sum_of_products(M const & m) -> M {
//     return m;
// }

// template <matcher M>
// auto sum_of_products(not_t<M> const & n) {
//     if constexpr (/* M is an and_t */) {
//         return or_t{
//             sum_of_products(negate(n.m.lhs)),
//             sum_of_products(negate(n.m.rhs))};
//     } else if constexpr (/* M is an or_t */) {
//         return sum_of_products(and_t{
//             sum_of_products(negate(n.m.lhs)),
//             sum_of_products(negate(n.m.rhs))});
//     } else {
//         return n;
//     }
// }

// template <matcher L, matcher R>
// auto sum_of_products(and_t<L, R> const & m) {
//     auto l = sum_of_products(m.lhs);
//     auto r = sum_of_products(m.rhs);

//     if constexpr (/* l is an or_t */) {
//         auto lr = sum_of_products(and_t{l.lhs, r});
//         auto rr = sum_of_products(and_t{l.rhs, r});
//         return or_t{lr, rr};
//     } else if constexpr (/* r is an or_t */) {
//         auto ll = sum_of_products(and_t{l, r.lhs});
//         auto lr = sum_of_products(and_t{l, r.rhs});
//         return or_t{ll, lr};
//     } else {
//         return and_t{l, r};
//     }
// }

// template <matcher L, matcher R>
// auto sum_of_products(or_t<L, R> const & m) {
//     auto l = sum_of_products(m.lhs);
//     auto r = sum_of_products(m.rhs);
//     return or_t{l, r};
// }
