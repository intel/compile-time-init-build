#pragma once


#include <cstdint>
#include <array>
#include <optional>
#include <utility>
#include <type_traits>

#include <boost/hana.hpp>


namespace hana = boost::hana;


/**
 * Ordered map that works well for integral keys whose values are very 'clumpy'. That is, there are long sequences
 * of keys that differ by a value of '1' with few gaps in a clump. There may be many clumps seperated by arbitray gaps.
 *
 * Tuning this map provides a trade-off in storage space and performance. Worst-case lookup time complexity is
 * O(log(n)) and best case is O(1). Absolute computational overhead is very low and is better than unordered and
 * ordered maps for clumpy key distributions.
 *
 * The map keys are immutable and must be initialized at compile-time as a constexpr or constinit instance. While
 * the keys are immutable and cannot be added or removed, values may be mutable if the map is declared as constinit.
 *
 * @tparam KeyTypeT
 * @tparam ValueTypeT
 * @tparam ClumpTypesT
 */
template<
    typename KeyTypeT,
    typename ValueTypeT,
    typename... ClumpTypesT>
struct ClumpyIntegralMap {
private:
    hana::tuple<ClumpTypesT...> clumps;

    template<
        typename T,
        typename BeginT,
        typename EndT>
    [[nodiscard]] constexpr std::optional<ValueTypeT> find(
        T const & list,
        BeginT begin,
        EndT end,
        int key
    ) const {
        auto const clumpsSize = hana::size(clumps);

        if constexpr ((begin <= end) && (end < clumpsSize)) {
            auto const guessIndex = (begin + end) / hana::size_c<2>;
            auto const & guess = clumps[guessIndex];

            if (guess > key) {
                return find(list, begin, guessIndex - hana::size_c<1>, key);
            } else if (guess < key) {
                return find(list, guessIndex + hana::size_c<1>, end, key);
            } else {
                return guess[key];
            }
        } else {
            return {};
        }
    }

public:
    constexpr ClumpyIntegralMap(
        ClumpTypesT... clumps
    )
        : clumps{clumps...}
    {}

    [[nodiscard]] constexpr std::optional<ValueTypeT> operator[](KeyTypeT key) const {
        return find(clumps, hana::size_c<0>, hana::size(clumps) - hana::size_c<1>, static_cast<int>(key));
    }
};

template<
    typename KeyTypeT,
    typename ValueTypeT,
    KeyTypeT baseKeyT,
    int sizeT>
struct Clump {
private:
    /// the first valid key in the clump
    constexpr static auto baseIndex = static_cast<int>(baseKeyT);

    /// the last valid key in the clump
    constexpr static auto endKey = baseIndex + sizeT - 1;

    /// storage for the contiguous values of the clump
    std::array<ValueTypeT, sizeT> storage;

public:
    constexpr Clump(
        std::initializer_list<std::pair<KeyTypeT, ValueTypeT>> pairs
    )
        : storage{}
    {
        for (auto const p : pairs) {
            auto const offset = static_cast<int>(p.first) - baseIndex;
            storage[offset] = p.second;
        }
    }

    [[nodiscard]] constexpr ValueTypeT operator[](int key) const {
        auto const offset = key - baseIndex;
        return storage[offset];
    }

    [[nodiscard]] constexpr bool operator<(int key) const {
        return endKey < key;
    }

    [[nodiscard]] constexpr bool operator>(int key) const {
        return baseIndex > key;
    }
};

template<
    typename KeyTypeT,
    typename ValueTypeT,
    typename... PairsT>
constexpr auto makeClumpyMap(PairsT... args) {
    // sort the incoming key/value pairs in ascending order by key
    auto const src =
        hana::sort(hana::make_tuple(args...), [](auto lhs, auto rhs){
            constexpr auto lhsValue = static_cast<int>(std::remove_reference_t<decltype(lhs[hana::size_c<0>])>::value);
            constexpr auto rhsValue = static_cast<int>(std::remove_reference_t<decltype(rhs[hana::size_c<0>])>::value);
            constexpr bool lhsLessThanRhs = lhsValue < rhsValue;
            return hana::bool_c<lhsLessThanRhs>;
        });

    auto const indices =
        hana::to<hana::tuple_tag>(hana::make_range(hana::size_c<1>, hana::size(src)));

    auto const initialState =
        hana::make_tuple(hana::make_tuple(src[hana::size_c<0>]));

    // split the flat tuple of pairs into a tuple of tuple of pairs according to gaps in the key
    auto const clumpTuples =
        hana::fold(indices, initialState, [&](auto clumps, auto index){
            auto const currentClump = hana::back(clumps);
            auto const prevPair = hana::back(currentClump);
            auto const prevKey = static_cast<int>(prevPair[hana::size_c<0>].value);
            auto const key = static_cast<int>(src[index][hana::size_c<0>].value);
            bool const gapPresent = (prevKey + 1) != key;

            if constexpr (gapPresent) {
                return hana::append(clumps, hana::make_tuple(src[index]));
            } else {
                auto const allButLastClump = hana::drop_back(clumps);
                return hana::append(allButLastClump, hana::append(currentClump, src[index]));
            }
        });

    // convert each contiguous tuple of pairs into a Clump
    auto const clumps =
        hana::transform(clumpTuples, [](auto clumpPairs){
            auto const clumpBaseKey = clumpPairs[hana::size_c<0>][hana::size_c<0>];
            auto const clumpSize = hana::size(clumpPairs);

            using ClumpType = Clump<KeyTypeT, ValueTypeT, clumpBaseKey, clumpSize>;

            return hana::unpack(clumpPairs, [](auto... pairs){
                return ClumpType{
                    {pairs[hana::size_c<0>], pairs[hana::size_c<1>]} ...
                };
            });
        });

    return hana::unpack(clumps, [](auto... clumpArgs){
        return ClumpyIntegralMap<KeyTypeT, ValueTypeT,
            decltype(clumpArgs) ...
        >{
            clumpArgs...
        };
    });
}


template<
    auto key,
    typename ValueTypeT>
constexpr auto clumpyPair(ValueTypeT value) {
    return hana::make_tuple(std::integral_constant<decltype(key), key>{}, value);
}