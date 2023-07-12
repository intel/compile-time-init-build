#pragma once

#include <msg/field_matchers.hpp>
#include <msg/match.hpp>
#include <sc/string_constant.hpp>

#include <cstdint>
#include <type_traits>

namespace msg {
// TODO: handle field types that are not integral types
/**
 * field class used to specify field lengths and manipulate desired field from
 * provided data
 * @tparam MsbT  most significant bit position for the field
 * @tparam LsbT  least significant bit position for the field
 */
template <typename NameTypeT, std::uint32_t DWordIndex, std::uint32_t MsbT,
          std::uint32_t LsbT, typename T = std::uint32_t, T DefaultValue = T{},
          typename MatchRequirementsType = match::always_t<true>>
class field {
  private:
    T value{DefaultValue};

  public:
    static_assert(LsbT <= MsbT, "msb needs to be lower than or equal to lsb");
    static_assert(LsbT <= 31, "lsb needs to be lower than or equal to 31");

    constexpr static size_t size = (MsbT - LsbT) + 1;
    static_assert(size <= 64, "field must be 64 bits or smaller");

    using FieldId = field<NameTypeT, DWordIndex, MsbT, LsbT, T>;
    using This = field<NameTypeT, DWordIndex, MsbT, LsbT, T, DefaultValue,
                       MatchRequirementsType>;
    using ValueType = T;

    using NameType = NameTypeT;
    constexpr static auto MaxDWordExtent = DWordIndex + (MsbT / 32);

    template <typename MsgType> constexpr static void fits_inside(MsgType) {
        static_assert(MaxDWordExtent < MsgType::max_num_dwords);
    }

    constexpr static NameType name{};
    constexpr static uint64_t bit_mask = [] {
        if constexpr (size == 64) {
            return 0xFFFFFFFFFFFFFFFFUL;
        } else {
            return (static_cast<uint64_t>(1) << static_cast<uint64_t>(size)) -
                   static_cast<uint64_t>(1);
        }
    }();

    constexpr static uint64_t field_mask = bit_mask << LsbT;

    constexpr static MatchRequirementsType match_requirements{};

    template <T expected_value>
    constexpr static msg::equal_to_t<This, T, expected_value> equal_to{};

    constexpr static msg::equal_to_t<This, T, DefaultValue> match_default{};

    template <T... expected_values>
    constexpr static msg::in_t<This, T, expected_values...> in{};

    template <T expected_value>
    constexpr static msg::greater_than_t<This, T, expected_value>
        greater_than{};

    template <T expected_value>
    constexpr static msg::greater_than_or_equal_to_t<This, T, expected_value>
        greater_than_or_equal_to{};

    template <T expected_value>
    constexpr static msg::less_than_t<This, T, expected_value> less_than{};

    template <T expected_value>
    constexpr static msg::less_than_or_equal_to_t<This, T, expected_value>
        less_than_or_equal_to{};

    template <T NewGreaterValue>
    using WithGreaterThan =
        field<NameTypeT, DWordIndex, MsbT, LsbT, T, NewGreaterValue,
              msg::greater_than_t<This, T, NewGreaterValue>>;

    template <T NewDefaultValue>
    using WithDefault =
        field<NameTypeT, DWordIndex, MsbT, LsbT, T, NewDefaultValue>;

    template <T NewRequiredValue>
    using WithRequired =
        field<NameTypeT, DWordIndex, MsbT, LsbT, T, NewRequiredValue,
              msg::equal_to_t<This, T, NewRequiredValue>>;

    template <T... PotentialValues>
    using WithIn = field<NameTypeT, DWordIndex, MsbT, LsbT, T, T{},
                         msg::in_t<This, T, PotentialValues...>>;

    template <typename NewRequiredMatcher>
    using WithMatch = field<NameTypeT, DWordIndex, MsbT, LsbT, T, DefaultValue,
                            NewRequiredMatcher>;

    template <T NewGreaterValue>
    using WithGreaterThanOrEqualTo =
        field<NameTypeT, DWordIndex, MsbT, LsbT, T, NewGreaterValue,
              msg::greater_than_or_equal_to_t<This, T, NewGreaterValue>>;

    template <T NewLesserValue>
    using WithLessThanOrEqualTo =
        field<NameTypeT, DWordIndex, MsbT, LsbT, T, NewLesserValue,
              msg::less_than_or_equal_to_t<This, T, NewLesserValue>>;

    constexpr explicit field(T const &new_value) : value{new_value} {}

    constexpr field() = default;

    template <typename DataType>
    [[nodiscard]] constexpr static auto extract(DataType const &data) -> T {
        std::uint32_t const lower = data[DWordIndex] >> LsbT;

        std::uint64_t const mid = [&] {
            if constexpr (MsbT < 32) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 1])
                       << (32 - LsbT);
            }
        }();

        std::uint64_t const upper = [&] {
            if constexpr (MsbT < 64) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 2])
                       << (64 - LsbT);
            }
        }();

        return static_cast<T>((upper | mid | lower) & bit_mask);
    }

    template <typename DataType> constexpr void insert(DataType &data) const {
        data[DWordIndex] = static_cast<std::uint32_t>(
            ((static_cast<std::uint32_t>(value) << LsbT) & field_mask) |
            (data[DWordIndex] & ~field_mask));

        if constexpr (MsbT >= 32) {
            constexpr auto field_mask_dword_1 =
                static_cast<std::uint32_t>(bit_mask >> (32 - LsbT));

            data[DWordIndex + 1] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(value) >> (32 - LsbT)) &
                 field_mask_dword_1) |
                (data[DWordIndex + 1] & ~field_mask_dword_1));
        }

        if constexpr (MsbT >= 64) {
            constexpr auto field_mask_dword_2 =
                static_cast<std::uint32_t>(bit_mask >> (64 - LsbT));

            data[DWordIndex + 2] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(value) >> (64 - LsbT)) &
                 field_mask_dword_2) |
                (data[DWordIndex + 2] & ~field_mask_dword_2));
        }
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{}: 0x{:x}"_sc, name, static_cast<std::uint32_t>(value));
    }
};
} // namespace msg
