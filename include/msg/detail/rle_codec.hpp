#pragma once
#include <lookup/entry.hpp>
#include <lookup/input.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/tuple.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <numeric>
#include <span>

namespace msg::detail {

template <std::size_t N>
using smallest_storage_type = decltype(stdx::detail::select_storage<N, void>());

// Captures RLE data for decoding and provides a mechanism to
// get a single-use enumerator to step through bits in the encoded
// bitset.
template <typename BitSetType> class rle_decoder {
  public:
    using bitset_type = BitSetType;
    constexpr static auto num_bits = BitSetType::size();

    constexpr explicit rle_decoder(std::byte const *start_rle_data)
        : rle_data{start_rle_data} {}

    // A type to allow "iteration" over the RLE encoded data in a way that
    // can be efficiently used to decode the runs of bits for intersection.
    //
    // This will return
    //  - a negative number for the number of consecutive 0's
    //  - a positive number for the number of consecutive 1's
    //  - zero if there are no more bits to decode
    //
    // Can traverse only a single time.
    class chunk_enumerator {
      public:
        constexpr static auto num_bits = BitSetType::size();

        constexpr chunk_enumerator() : rle_data{nullptr} {}

        constexpr explicit chunk_enumerator(std::byte const *start_rle_data)
            : rle_data{start_rle_data} {
            next_chunk();
        }

        // Get the current chunk of continuous bits. -ve values are 0s
        // +ve values are 1s. range is -255...255. will return 0 if finished
        [[nodiscard]] constexpr auto bit_chunk() const -> std::int_fast16_t {
            return bit_value ? current_run : -current_run;
        }

        // Advance the bit chunk by `bits` number of bits. This might consume
        // only a portion of the remain bits in the chunk, skip to the next
        // chunk or skip over multiple chunks. The current bit position in the
        // data stream must be provided by caller to avoid reading past end of
        // RLE data. We rely on the caller to avoid needing multiple bit
        // counters when there are multiple chunk_enumerators in play.
        //
        // Returns the new current_bit (or num_bits if we pass the end of the
        // data) after the bits are consumed.
        constexpr auto advance(std::size_t bits, std::size_t current_bit)
            -> std::size_t {
            while (bits > 0 && current_bit < num_bits) {
                // more available than we are consuming?
                if (bits < current_run) {
                    current_run -= static_cast<std::uint8_t>(bits);
                    return current_bit + bits;
                }

                // consume all the currently available bits
                // and get the next chunk
                bits -= current_run;
                current_bit += current_run;
                // only load next chunk of we are not at the end
                if (current_bit < num_bits) {
                    next_chunk();
                } else {
                    // no more bits.
                    current_run = 0;
                }
            }

            return current_bit;
        }

      private:
        std::byte const *rle_data;
        // initial load values so the first next_chunk() call in constructor
        // starts in the 0's state (pretend we just did some 1s).
        std::uint8_t current_run{0};
        bool bit_value{true};

        constexpr void next_chunk() {
            // skipping the next bit count for a > 255 run?
            if (*rle_data == std::byte{0}) {
                // keep same bit_value and skip this encoded byte
                ++rle_data;
            } else {
                // invert bit_value to generate run of opposite bits
                bit_value = not bit_value;
            }
            current_run = static_cast<std::uint8_t>(*rle_data++);
        }
    };

    [[nodiscard]] constexpr auto make_chunk_enumerator() const
        -> chunk_enumerator {
        return chunk_enumerator{rle_data};
    }

  private:
    std::byte const *rle_data;
};

template <typename BitSetType> struct rle_codec {
    using bitset_type = BitSetType;
    constexpr static auto const num_bits = BitSetType::size();

    // assume worst case of each bitmap being alternating bit values
    using max_rle_data_type = stdx::cx_vector<std::byte, num_bits * 2>;

    constexpr static auto encode(bitset_type const &bitset)
        -> max_rle_data_type {
        max_rle_data_type data{};
        std::size_t count{0};
        bool last{false};

        for (std::size_t bit{0}; bit < num_bits; ++bit) {
            if (bitset[bit] != last) {
                data.push_back(static_cast<std::byte>(count));
                last = !last;
                count = 1;
            } else {
                if (++count > 255) {
                    // we have overflowed the max byte. we need to
                    // reset our count and indicate there are no alternate bits
                    data.push_back(std::byte{255});
                    data.push_back(std::byte{0});
                    count = 1;
                }
            }
        }
        // final byte
        data.push_back(static_cast<std::byte>(count));
        return data;
    }

    constexpr static auto decode(std::byte const *rle_data) -> bitset_type {
        bitset_type result{};

        auto decoder =
            rle_decoder<BitSetType>{rle_data}.make_chunk_enumerator();
        std::size_t bit{0};

        while (bit < decoder.num_bits) {
            auto chunk_bits = decoder.bit_chunk();
            if (chunk_bits < 0) {
                // skip 0's
                bit =
                    decoder.advance(static_cast<std::size_t>(-chunk_bits), bit);
            } else {
                auto temp_bit = bit;
                bit =
                    decoder.advance(static_cast<std::size_t>(chunk_bits), bit);

                // add the 1s
                while (chunk_bits-- > 0) {
                    result.set(temp_bit++);
                }
            }
        }

        return result;
    }
};

template <typename BitSetType,
          std::same_as<rle_decoder<BitSetType>>... Decoders>
    requires(sizeof...(Decoders) > 0)
struct rle_intersect {
    using bitset_type = BitSetType;
    using decoder_type = rle_decoder<BitSetType>;
    using chunk_type = typename decoder_type::chunk_enumerator;
    constexpr static auto const num_bits = decoder_type::num_bits;
    constexpr static auto const num_decoders = sizeof...(Decoders);

    std::array<decoder_type, num_decoders> decoder_list;

    constexpr explicit rle_intersect(Decoders &&...decoders)
        : decoder_list{std::forward<Decoders>(decoders)...} {}

    // iterate over set bits, passing them to bool (&f)(auto bit_number).
    // if f returns true, we abort and return true indicating early exit
    // otherwise return false to indicate no early abort of iteration
    template <typename F> constexpr auto for_each_until(F &&f) const -> bool {
        // allocate a set of chunk_enumerators for each decoder
        // so that we can traverse the bit set intersection
        stdx::cx_vector<chunk_type, num_decoders> chunks{};
        for (auto &d : decoder_list) {
            chunks.push_back(d.make_chunk_enumerator());
        }

        // advance all chunks by a number of bits and return the new
        // current bit position
        auto advance = [&](std::size_t bits,
                           std::size_t current_bit) -> std::size_t {
            std::size_t new_current{current_bit};
            for (auto &c : chunks) {
                new_current = c.advance(bits, current_bit);
            }
            return new_current;
        };

        // comparison of chunk bit counts
        auto min_chunk = [](chunk_type const &a, chunk_type const &b) -> bool {
            return a.bit_chunk() < b.bit_chunk();
        };

        std::size_t bit{0};
        while (bit < num_bits) {
            // the min "bit_chunk" item in the chunk list is the smallest run
            // length chunk value. If that value is -ve this is a run of zeros,
            // and so we can immediately consume that many bits because
            // 0 and X = 0. Otherwise, if +ve, it will be the smallest number of
            // consecutive 1s, and all other chunks will contain more 1s, and so
            // we can add that many ones to the result.
            //
            // NOTE: zero length chunks are ignored as the bits counter will end
            //       the loop as each rle_decoder finishes at the same time
            auto min_chunk_bits =
                (*std::min_element(chunks.begin(), chunks.end(), min_chunk))
                    .bit_chunk();

            if (min_chunk_bits > 0) {
                // this will be the minimum number of 1s and all other
                // chunks must also be 1s.
                auto temp_bit = bit;
                bit = advance(static_cast<std::size_t>(min_chunk_bits), bit);
                while (min_chunk_bits-- > 0) {
                    // call F, but abort if it indicates abort requested
                    if (f(temp_bit++)) {
                        return true; // early abort
                    }
                }
            } else {
                // otherwise it was the maximum run of 0s no need to invoke F
                bit = advance(static_cast<std::size_t>(-min_chunk_bits), bit);
            }
        }
        return false; // full traversal
    }

    template <typename F> constexpr auto for_each(F &&f) const -> F {
        for_each_until([&](auto i) {
            f(i);
            return false;
        });
        return std::forward<F>(f);
    }

    template <typename F>
    friend constexpr auto for_each(F &&f, rle_intersect const &intersect) -> F {
        return intersect.for_each(std::forward<F>(f));
    }

    [[nodiscard]] constexpr auto any() const -> bool {
        // iterate until we find at least a single bit.
        return for_each_until([](auto /*unused*/) { return true; });
    }

    [[nodiscard]] constexpr auto none() const -> bool { return not any(); }

    [[nodiscard]] constexpr auto get_bitset() const -> bitset_type {
        bitset_type result{};

        for_each([&](auto i) { result.set(i); });

        return result;
    }
};

// at least 1 decoder is required
template <typename Decoder, std::same_as<Decoder>... Others>
rle_intersect(Decoder d, Others... others)
    -> rle_intersect<typename Decoder::bitset_type, Decoder, Others...>;

} // namespace msg::detail
