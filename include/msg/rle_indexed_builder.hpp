#pragma once

#include <log/log.hpp>
#include <msg/detail/indexed_builder_common.hpp>
#include <msg/detail/rle_codec.hpp>
#include <msg/rle_indexed_handler.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace msg {

namespace detail {

// Used to build all the individual RLE encoded bitset sequences
// for a given index, including the default_value.
template <typename FieldType, typename KeyType, typename BitSetType,
          std::size_t NumBitSets>
struct rle_index_raw_encoding {
    using field_type = FieldType;
    using key_type = KeyType;
    using bitset_type = BitSetType;
    using codec_type = rle_codec<BitSetType>;
    using max_rle_data_type = typename codec_type::max_rle_data_type;

    constexpr static auto num_bits = codec_type::num_bits;
    constexpr static auto num_bitsets = NumBitSets;

    // we want to store RLE data for each bitmap, indexed by the index key
    using rle_index_data_type =
        stdx::cx_map<key_type, max_rle_data_type, num_bitsets>;

    constexpr explicit rle_index_raw_encoding(
        BitSetType const &the_default_value)
        : default_value{codec_type::encode(the_default_value)} {}

    max_rle_data_type default_value;
    rle_index_data_type rle_index{};
};

// Storage for an offset into an RLE encoded blob of a specific bitset
// for the given field type. OffsetType is the smallest type needed to
// offset into the full RLE blob.
//
// Note: the LockType is used to ensure that rle_index and rle_storage match
template <typename LockType, typename FieldType, typename BitSetType,
          typename OffsetType>
struct rle_index {
    using field_type = FieldType;
    using bitset_type = BitSetType;
    using offset_type = OffsetType;

    offset_type offset;

    constexpr rle_index() = default;
    constexpr explicit rle_index(offset_type the_offset) : offset{the_offset} {}
};

// Storage for all the RLE encoded data with an accessor to decode an
// rle_index into the appropriate bitset
//
// Note: the LockType is used to ensure that rle_index and rle_storage match
template <typename LockType, std::size_t DataLength> struct rle_storage {
    using storage_type = std::array<std::byte, DataLength>;

    // get a bitset from an rle_index
    template <typename FieldType, typename BitSetType, typename OffsetType>
    constexpr auto
    get(rle_index<LockType, FieldType, BitSetType, OffsetType> idx) const
        -> BitSetType {
        using codec_type = rle_codec<BitSetType>;
        return codec_type::decode(std::next(data.begin(), idx.offset));
    }

    template <typename FieldType, typename BitSetType, typename OffsetType>
    constexpr auto
    decode(rle_index<LockType, FieldType, BitSetType, OffsetType> idx) const
        -> rle_decoder<BitSetType> {
        return rle_decoder<BitSetType>{std::next(data.begin(), idx.offset)};
    }

    storage_type data;
};

// Build the encoded RLE data with a max length of MaxDataLen
// Take the opportunity to reuse byte sequences where possible.
template <std::size_t MaxDataLength> struct rle_storage_builder {
    using offset_type = detail::smallest_storage_type<MaxDataLength>;

    // add the given data into the storage, attempting to reuse previous
    template <typename Data>
    constexpr auto add(Data const &data) -> offset_type {
        // check for existing matching data to reuse
        auto found = std::search(std::begin(storage), std::end(storage),
                                 std::begin(data), std::end(data));
        if (found != storage.end()) {
            // reuse existing data location
            return static_cast<offset_type>(
                std::distance(std::begin(storage), found));
        }

        // grab the current location and copy into the storage
        auto const location = storage_location;

        auto const next_location =
            std::copy(std::begin(data), std::end(data),
                      std::next(std::begin(storage), storage_location));

        storage_location = std::distance(std::begin(storage), next_location);

        return static_cast<offset_type>(location);
    }

    // build a temp index of key -> offset and collect all rle data
    // into the storage builder
    template <typename OffsetT, typename Idx>
    constexpr auto add_index(Idx idx) {
        constexpr auto num_entries = idx.num_bitsets;

        // needs to support stdx::make_indexed_tuple<get_field_type>
        struct storage_lookup_index {
            using field_type [[maybe_unused]] = typename Idx::field_type;
            using bitset_type [[maybe_unused]] = typename Idx::bitset_type;
            using key_type = typename Idx::key_type;
            using entry_type = lookup::entry<key_type, OffsetT>;

            constexpr explicit storage_lookup_index(OffsetT the_default_value)
                : default_value{the_default_value} {}

            OffsetT default_value;
            std::array<entry_type, num_entries> entries{};
        };

        storage_lookup_index result{add(idx.default_value)};

        std::transform(std::begin(idx.rle_index), std::end(idx.rle_index),
                       std::begin(result.entries), [&](auto const &kvp) {
                           return lookup::entry{kvp.key, add(kvp.value)};
                       });

        return result;
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return static_cast<std::size_t>(storage_location);
    }

    // build a truncated copy of the storage accounting for the savings of
    // reusing bitset RLE patterns
    template <std::size_t FinalSize>
    constexpr auto truncate() const -> std::array<std::byte, FinalSize> {
        std::array<std::byte, FinalSize> result;
        std::copy_n(std::begin(storage), FinalSize, std::begin(result));
        return result;
    }

    std::array<std::byte, MaxDataLength> storage{};
    std::ptrdiff_t storage_location{};
};

} // namespace detail

// TODO: needs index configuration
template <typename IndexSpec, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct rle_indexed_builder
    : indexed_builder_base<rle_indexed_builder, IndexSpec, CallbacksT, BaseMsgT,
                           ExtraCallbackArgsT...> {
    using base_t =
        indexed_builder_base<rle_indexed_builder, IndexSpec, CallbacksT,
                             BaseMsgT, ExtraCallbackArgsT...>;

    // encode the index I into a rle_index_raw_encoding
    template <typename BuilderValue, typename Idx>
    static CONSTEVAL auto raw_encode_index() {
        constexpr auto temp_indices =
            base_t::template create_temp_indices<BuilderValue>();

        using field_type = typename Idx::field_type;
        using bitset_type = decltype(get<Idx>(temp_indices).default_value);
        using index_key_type =
            typename decltype(get<Idx>(temp_indices).entries)::key_type;
        constexpr auto num_bitsets = get<Idx>(temp_indices).entries.size();

        using result_type =
            detail::rle_index_raw_encoding<field_type, index_key_type,
                                           bitset_type, num_bitsets>;
        using codec_type = typename result_type::codec_type;

        auto const default_value = get<Idx>(temp_indices).default_value;

        result_type result{default_value};

        for (auto const &kvp : get<Idx>(temp_indices).entries) {
            auto const key = kvp.key;
            // merge the index value with the default to ensure that
            // unfiltered callbacks are always called.
            auto const bs = kvp.value | default_value;
            result.rle_index.put(key, codec_type::encode(bs));
        }

        return result;
    }

    // build a full index with RLE storage. The storage here is
    // allocated worst case, but is likely smaller due to reuse of data.
    // It will be shrunk and transformed later
    template <typename BuilderValue, std::size_t MaxRleLength>
    static CONSTEVAL auto make_full_rle_index() {
        constexpr auto temp_indices =
            base_t::template create_temp_indices<BuilderValue>();

        constexpr auto all_rle_data =
            temp_indices.apply([]<typename... I>(I...) {
                // build a replacement tuple of indices, but with raw RLE
                // encoding
                return stdx::make_indexed_tuple<get_field_type>(
                    raw_encode_index<BuilderValue, I>()...);
            });

        return all_rle_data.apply([]<typename... I>(I... indices) {
            using rle_storage_type = detail::rle_storage_builder<MaxRleLength>;

            using initial_offset_type =
                typename detail::rle_storage_builder<MaxRleLength>::offset_type;

            rle_storage_type storage{};

            auto full_encoded_indices =
                stdx::make_indexed_tuple<get_field_type>(
                    storage.template add_index<initial_offset_type>(
                        indices)...);

            struct full_encoded_result {
                decltype(full_encoded_indices) lookup;
                rle_storage_type storage;
            };

            return full_encoded_result{full_encoded_indices, storage};
        });
    }

    template <typename LockType, std::size_t FinalRleLength, typename Idx>
    constexpr static auto re_encode_index(Idx idx) {
        // idx is a storage_lookup_index from above

        // recalculate the offset type based on the final rle length
        // after reusing RLE patterns
        using offset_type = detail::smallest_storage_type<FinalRleLength>;

        constexpr auto num_entries = idx.entries.size();

        // needs to support stdx::make_indexed_tuple<get_field_type>
        struct field_lookup_index {
            using field_type [[maybe_unused]] = typename Idx::field_type;
            using bitset_type [[maybe_unused]] = typename Idx::bitset_type;
            using key_type [[maybe_unused]] = typename Idx::key_type;
            using index_type [[maybe_unused]] =
                detail::rle_index<LockType, field_type, bitset_type,
                                  offset_type>;

            constexpr explicit field_lookup_index(index_type the_default_value)
                : lookup_input{the_default_value} {}

            lookup::input<key_type, index_type, num_entries> lookup_input;
        };

        using index_type = typename field_lookup_index::index_type;

        // initialise the resulting index and then transform
        // the passed in index into the final form using the correct
        // rle_index proxy objects
        field_lookup_index result{index_type{idx.default_value}};
        std::transform(
            std::begin(idx.entries), std::end(idx.entries),
            std::begin(result.lookup_input.entries), [&](auto const &entry) {
                return lookup::entry{entry.key_, index_type{entry.value_}};
            });

        return result;
    }

    template <typename LockType, std::size_t FinalRleLength, typename FullRleT>
    static CONSTEVAL auto
    shrink_and_transform_encoded_result(FullRleT const &full_rle) {
        // remap the indices into the final lookup type ready to be encoded
        // into the message decoding indices
        auto re_encoded_lookup =
            full_rle.lookup.apply([]<typename... I>(I... indices) {
                return stdx::make_indexed_tuple<get_field_type>(
                    re_encode_index<LockType, FinalRleLength>(indices)...);
            });

        struct encoded_result {
            decltype(re_encoded_lookup) lookup;
            detail::rle_storage<LockType, FinalRleLength> storage;
        };

        return encoded_result{
            re_encoded_lookup,
            full_rle.storage.template truncate<FinalRleLength>()};
    }

    template <typename BuilderValue> static CONSTEVAL auto make_rle_index() {
        // lock the rle_index and rle_storage types for this BuilderValue
        // so that they can't be misused by other instances.
        struct access_lock {};

        constexpr auto temp_indices =
            base_t::template create_temp_indices<BuilderValue>();

        constexpr auto all_rle_data =
            temp_indices.apply([]<typename... I>(I...) {
                // build a replacement tuple of indices, but with raw RLE
                // encoding
                return stdx::make_indexed_tuple<get_field_type>(
                    raw_encode_index<BuilderValue, I>()...);
            });

        // compute the total length of all RLE data which we will use as
        // the worst case encoding length
        constexpr auto total_rle_data_length =
            all_rle_data.apply([=]<typename... I>(I... indices) -> std::size_t {
                // count all the lengths of the rle encoded values for the
                // index
                constexpr auto count_index_len = []<typename Idx>(Idx idx) {
                    return std::accumulate(std::begin(idx), std::end(idx),
                                           std::size_t{0},
                                           [](auto sum, auto &kvp) {
                                               return sum + kvp.value.size();
                                           });
                };

                // sum all the lengths of the rle index data and the lengths
                // of the default values.
                return (count_index_len(indices.rle_index) + ...) +
                       (indices.default_value.size() + ...);
            });

        // build the combined RLE data for all bitsets, and a tuple of
        // lookup indices into that RLE data for each index and key in that
        // index.
        constexpr auto full_encoded_rle_indices =
            make_full_rle_index<BuilderValue, total_rle_data_length>();

        constexpr auto final_rle_length =
            full_encoded_rle_indices.storage.size();

        // shrink the storage and transform into lookup ready index types
        return shrink_and_transform_encoded_result<access_lock,
                                                   final_rle_length>(
            full_encoded_rle_indices);
    }

    template <typename BuilderValue, typename I>
    static CONSTEVAL auto make_rle_input() {
        struct {
            CONSTEVAL auto operator()() const noexcept {
                constexpr auto rle = make_rle_index<BuilderValue>();
                return get<typename I::field_type>(rle.lookup).lookup_input;
            }
            using cx_value_t [[maybe_unused]] = void;
        } val;
        return val;
    }

    template <typename BuilderValue> static CONSTEVAL auto build() {
        constexpr auto temp_indices =
            base_t::template create_temp_indices<BuilderValue>();

        constexpr auto rle = make_rle_index<BuilderValue>();

        constexpr auto baked_indices =
            temp_indices.apply([=]<typename... I>(I...) {
                return rle_indices{
                    rle.storage,
                    index{typename I::field_type{},
                          lookup::make(make_rle_input<BuilderValue, I>())}...};
            });

        constexpr auto num_callbacks = BuilderValue::value.callbacks.size();

        constexpr auto callback_array =
            base_t::template create_callback_array<BuilderValue>(
                std::make_index_sequence<num_callbacks>{});

        return indexed_handler{
            callback_args_t<BaseMsgT, ExtraCallbackArgsT...>{}, baked_indices,
            callback_array};
    }
};

} // namespace msg
