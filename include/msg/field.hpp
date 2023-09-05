#pragma once

#include <msg/field_matchers.hpp>
#include <msg/match.hpp>
#include <sc/format.hpp>

#include <concepts>
#include <cstdint>
#include <span>
#include <type_traits>

namespace msg {
// TODO: handle field types that are not integral types

template <typename T>
concept field_spec =
    std::unsigned_integral<decltype(T::size)> and
    (std::integral<typename T::type> or std::is_enum_v<typename T::type>) and
    requires {
        typename T::name_t;
        { T::bit_mask } -> std::same_as<std::uint64_t const &>;
    };

template <typename T>
concept field_value = field_spec<T> and requires(T t) {
    { t.value } -> std::same_as<typename T::type &>;
};

template <typename T, typename Spec>
concept field_extractor_for =
    requires(std::span<std::uint32_t const> extract_buffer) {
        {
            T::template extract<Spec>(extract_buffer)
        } -> std::same_as<typename Spec::type>;
    };

template <typename T, typename Field>
concept field_inserter_for =
    requires(std::span<std::uint32_t> insert_buffer, Field f) {
        { T::template insert<Field>(insert_buffer, f) } -> std::same_as<void>;
    };

template <typename T, typename Field>
concept field_locator_for =
    field_extractor_for<T, Field> and field_inserter_for<T, Field>;

template <typename Name, typename T, std::uint32_t BitSize>
    requires(BitSize <= 64)
struct field_spec_t {
    using type = T;
    using name_t = Name;

    constexpr static name_t name{};
    constexpr static auto size = BitSize;

    constexpr static uint64_t bit_mask = [] {
        if constexpr (BitSize == 64) {
            return 0xFFFFFFFFFFFFFFFFUL;
        } else {
            return (static_cast<uint64_t>(1)
                    << static_cast<uint64_t>(BitSize)) -
                   static_cast<uint64_t>(1);
        }
    }();
};

template <std::uint32_t DWordIndex, std::uint32_t Lsb> struct field_locator_t {
    template <field_spec Spec>
    [[nodiscard]] constexpr static auto
    extract(std::span<std::uint32_t const> data) -> typename Spec::type {
        std::uint32_t const lower = data[DWordIndex] >> Lsb;
        constexpr auto Msb = Lsb + Spec::size - 1;

        std::uint64_t const mid = [&] {
            if constexpr (Msb < 32) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 1])
                       << (32 - Lsb);
            }
        }();

        std::uint64_t const upper = [&] {
            if constexpr (Msb < 64) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 2])
                       << (64 - Lsb);
            }
        }();

        return static_cast<typename Spec::type>((upper | mid | lower) &
                                                Spec::bit_mask);
    }

    template <field_value Field>
    constexpr static auto insert(std::span<std::uint32_t> data, Field f)
        -> void {
        constexpr std::uint64_t field_mask = Field::bit_mask << Lsb;
        constexpr auto Msb = Lsb + Field::size - 1;

        data[DWordIndex] = static_cast<std::uint32_t>(
            ((static_cast<std::uint32_t>(f.value) << Lsb) & field_mask) |
            (data[DWordIndex] & ~field_mask));

        if constexpr (Msb >= 32) {
            constexpr auto field_mask_dword_1 =
                static_cast<std::uint32_t>(Field::bit_mask >> (32 - Lsb));

            data[DWordIndex + 1] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(f.value) >> (32 - Lsb)) &
                 field_mask_dword_1) |
                (data[DWordIndex + 1] & ~field_mask_dword_1));
        }

        if constexpr (Msb >= 64) {
            constexpr auto field_mask_dword_2 =
                static_cast<std::uint32_t>(Field::bit_mask >> (64 - Lsb));

            data[DWordIndex + 2] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(f.value) >> (64 - Lsb)) &
                 field_mask_dword_2) |
                (data[DWordIndex + 2] & ~field_mask_dword_2));
        }
    }

    template <field_spec Spec, typename MsgType>
    constexpr static auto fits_inside() -> bool {
        constexpr auto max_dword_index =
            DWordIndex + (Lsb + Spec::size - 1) / 32;
        return max_dword_index < MsgType::max_num_dwords;
    }
};

/**
 * field class used to specify field lengths and manipulate desired
 * field from provided data
 * @tparam Msb  most significant bit position for the field
 * @tparam Lsb  least significant bit position for the field
 */
template <typename Name, std::uint32_t DWordIndex, std::uint32_t Msb,
          std::uint32_t Lsb, typename T = std::uint32_t, T DefaultValue = T{},
          typename MatchRequirementsType = match::always_t<true>>
    requires(Lsb <= 31 and Lsb <= Msb)
class field : public field_spec_t<Name, T, Msb - Lsb + 1>,
              public field_locator_t<DWordIndex, Lsb> {
    using spec_t = field_spec_t<Name, T, Msb - Lsb + 1>;
    using locator_t = field_locator_t<DWordIndex, Lsb>;
    friend locator_t;

    T value{DefaultValue};

  public:
    using FieldId = field<Name, DWordIndex, Msb, Lsb, T>;
    using ValueType = T;
    constexpr static auto MaxDWordExtent = DWordIndex + (Msb / 32);

    template <typename DataType>
    [[nodiscard]] constexpr static auto extract(DataType const &data) -> T {
        static_assert(
            std::same_as<typename DataType::value_type, std::uint32_t>);
        return locator_t::template extract<spec_t>(
            std::span{data.begin(), data.end()});
    }

    template <typename DataType> constexpr void insert(DataType &data) const {
        static_assert(
            std::same_as<typename DataType::value_type, std::uint32_t>);
        locator_t::insert(std::span{data.begin(), data.end()}, *this);
    }

    template <typename MsgType>
    constexpr static void fits_inside(MsgType const &) {
        static_assert(locator_t::template fits_inside<spec_t, MsgType>());
    }

    constexpr static MatchRequirementsType match_requirements{};

    template <T expected_value>
    constexpr static msg::equal_to_t<field, T, expected_value> equal_to{};

    constexpr static msg::equal_to_t<field, T, DefaultValue> match_default{};

    template <T... expected_values>
    constexpr static msg::in_t<field, T, expected_values...> in{};

    template <T expected_value>
    constexpr static msg::greater_than_t<field, T, expected_value>
        greater_than{};

    template <T expected_value>
    constexpr static msg::greater_than_or_equal_to_t<field, T, expected_value>
        greater_than_or_equal_to{};

    template <T expected_value>
    constexpr static msg::less_than_t<field, T, expected_value> less_than{};

    template <T expected_value>
    constexpr static msg::less_than_or_equal_to_t<field, T, expected_value>
        less_than_or_equal_to{};

    template <T NewGreaterValue>
    using WithGreaterThan =
        field<Name, DWordIndex, Msb, Lsb, T, NewGreaterValue,
              msg::greater_than_t<field, T, NewGreaterValue>>;

    template <T NewDefaultValue>
    using WithDefault = field<Name, DWordIndex, Msb, Lsb, T, NewDefaultValue>;

    template <T NewRequiredValue>
    using WithRequired = field<Name, DWordIndex, Msb, Lsb, T, NewRequiredValue,
                               msg::equal_to_t<field, T, NewRequiredValue>>;

    template <T... PotentialValues>
    using WithIn = field<Name, DWordIndex, Msb, Lsb, T, T{},
                         msg::in_t<field, T, PotentialValues...>>;

    template <typename NewRequiredMatcher>
    using WithMatch =
        field<Name, DWordIndex, Msb, Lsb, T, DefaultValue, NewRequiredMatcher>;

    template <T NewGreaterValue>
    using WithGreaterThanOrEqualTo =
        field<Name, DWordIndex, Msb, Lsb, T, NewGreaterValue,
              msg::greater_than_or_equal_to_t<field, T, NewGreaterValue>>;

    template <T NewLesserValue>
    using WithLessThanOrEqualTo =
        field<Name, DWordIndex, Msb, Lsb, T, NewLesserValue,
              msg::less_than_or_equal_to_t<field, T, NewLesserValue>>;

    constexpr field() = default;
    constexpr explicit(true) field(T const &new_value) : value{new_value} {}

    [[nodiscard]] constexpr auto describe() const {
        return format("{}: 0x{:x}"_sc, spec_t::name,
                      static_cast<std::uint32_t>(value));
    }
};
} // namespace msg
