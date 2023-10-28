#pragma once

#include <match/ops.hpp>
#include <msg/field_matchers.hpp>
#include <sc/format.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

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
concept bits_extractor =
    std::unsigned_integral<decltype(T::size)> and
    requires(std::span<std::uint32_t const> extract_buffer) {
        { T::extract(extract_buffer) } -> std::same_as<std::uint64_t>;
    };

template <typename T>
concept bits_inserter =
    std::unsigned_integral<decltype(T::size)> and
    requires(std::span<std::uint32_t> insert_buffer, std::uint64_t v) {
        { T::insert(insert_buffer, v) } -> std::same_as<void>;
    };

template <typename T>
concept bits_locator = bits_extractor<T> and bits_inserter<T>;

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

namespace detail {
template <std::uint32_t BitSize>
    requires(BitSize <= 64)
CONSTEVAL auto bit_mask() -> std::uint64_t {
    if constexpr (BitSize == 64) {
        return 0xFFFF'FFFF'FFFF'FFFFUL;
    } else {
        return (static_cast<std::uint64_t>(1)
                << static_cast<std::uint64_t>(BitSize)) -
               static_cast<std::uint64_t>(1);
    }
}
} // namespace detail

template <typename Name, typename T, std::uint32_t BitSize>
struct field_spec_t {
    using type = T;
    using name_t = Name;

    constexpr static name_t name{};
    constexpr static auto size = BitSize;
    constexpr static uint64_t bit_mask = detail::bit_mask<BitSize>();
};

template <std::uint32_t DWordIndex, std::uint32_t BitSize, std::uint32_t Lsb>
struct bits_locator_t {
    constexpr static auto size = BitSize;

    [[nodiscard]] constexpr static auto fold(std::uint64_t value)
        -> std::uint64_t {
        if constexpr (BitSize == 64) {
            return {};
        } else {
            return value >> BitSize;
        }
    }

    [[nodiscard]] constexpr static auto unfold(std::uint64_t value)
        -> std::uint64_t {
        if constexpr (BitSize == 64) {
            return {};
        } else {
            return value << BitSize;
        }
    }

    [[nodiscard]] constexpr static auto
    extract(std::span<std::uint32_t const> data) -> std::uint64_t {
        constexpr auto Msb = Lsb + BitSize - 1;
        std::uint32_t const lower = data[DWordIndex] >> Lsb;
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
        return (upper | mid | lower) & detail::bit_mask<BitSize>();
    }

    constexpr static auto insert(std::span<std::uint32_t> data,
                                 std::uint64_t const &value) -> void {
        constexpr auto Msb = Lsb + BitSize - 1;
        constexpr std::uint64_t mask = detail::bit_mask<BitSize>() << Lsb;
        data[DWordIndex] = static_cast<std::uint32_t>(
            ((static_cast<std::uint32_t>(value) << Lsb) & mask) |
            (data[DWordIndex] & ~mask));

        if constexpr (Msb >= 32) {
            constexpr auto mask_dword_1 = static_cast<std::uint32_t>(
                detail::bit_mask<BitSize>() >> (32 - Lsb));

            data[DWordIndex + 1] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(value) >> (32 - Lsb)) &
                 mask_dword_1) |
                (data[DWordIndex + 1] & ~mask_dword_1));
        }
        if constexpr (Msb >= 64) {
            constexpr auto field_mask_dword_2 = static_cast<std::uint32_t>(
                detail::bit_mask<BitSize>() >> (64 - Lsb));

            data[DWordIndex + 2] = static_cast<std::uint32_t>(
                ((static_cast<std::uint64_t>(value) >> (64 - Lsb)) &
                 field_mask_dword_2) |
                (data[DWordIndex + 2] & ~field_mask_dword_2));
        }
    }

    template <std::uint32_t MaxDwords>
    constexpr static auto fits_inside() -> bool {
        constexpr auto max_dword_index = DWordIndex + (Lsb + BitSize - 1) / 32;
        return max_dword_index < MaxDwords;
    }
};

template <bits_locator... BLs> struct field_locator_t {
    template <field_spec Spec>
    [[nodiscard]] constexpr static auto
    extract(std::span<std::uint32_t const> data) -> typename Spec::type {
        auto raw = std::uint64_t{};
        auto const extract_bits = [&]<bits_locator B>() {
            raw = B::unfold(raw);
            raw |= B::extract(data);
        };
        (..., extract_bits.template operator()<BLs>());
        return static_cast<typename Spec::type>(raw);
    }

    template <field_spec Spec>
    constexpr static auto insert(std::span<std::uint32_t> data,
                                 typename Spec::type const &value) -> void {
        auto raw = static_cast<std::uint64_t>(value);
        auto const insert_bits = [&]<bits_locator B>() {
            B::insert(data, raw & detail::bit_mask<B::size>());
            raw = B::fold(raw);
        };

        [[maybe_unused]] int dummy{};
        (void)(dummy = ... = (insert_bits.template operator()<BLs>(), 0));
    }

    template <std::uint32_t MaxDwords>
    constexpr static auto fits_inside() -> bool {
        return (... and BLs::template fits_inside<MaxDwords>());
    }
};

enum struct dword_index_t : std::uint32_t {};
enum struct msb_t : std::uint32_t {};
enum struct lsb_t : std::uint32_t {};

// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_dw(unsigned long long int v) -> dword_index_t {
    return static_cast<dword_index_t>(v);
}
// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_msb(unsigned long long int v) -> msb_t {
    return static_cast<msb_t>(v);
}
// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_lsb(unsigned long long int v) -> lsb_t {
    return static_cast<lsb_t>(v);
}

struct at {
    dword_index_t index_{};
    msb_t msb_{};
    lsb_t lsb_{};

    [[nodiscard]] constexpr auto index() const
        -> std::underlying_type_t<dword_index_t> {
        return stdx::to_underlying(index_);
    }
    [[nodiscard]] constexpr auto msb() const -> std::underlying_type_t<msb_t> {
        return stdx::to_underlying(msb_);
    }
    [[nodiscard]] constexpr auto lsb() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(lsb_);
    }

    [[nodiscard]] constexpr auto size() const -> std::underlying_type_t<lsb_t> {
        return msb() - lsb() + 1;
    }
    [[nodiscard]] constexpr auto dword_extent() const
        -> std::underlying_type_t<dword_index_t> {
        return index() + (msb() / 32);
    }
};

namespace detail {
template <at... Ats>
using locator_for =
    field_locator_t<bits_locator_t<Ats.index(), Ats.size(), Ats.lsb()>...>;

template <at... Ats> constexpr inline auto field_size = (0u + ... + Ats.size());

template <at... Ats>
constexpr inline auto max_dword_extent = std::max({0u, Ats.dword_extent()...});
} // namespace detail

template <typename Name, typename T = std::uint32_t, T DefaultValue = T{},
          match::matcher M = match::always_t, at... Ats>
class field_t : public field_spec_t<Name, T, detail::field_size<Ats...>>,
                public detail::locator_for<Ats...> {
    using spec_t = field_spec_t<Name, T, detail::field_size<Ats...>>;
    using locator_t = detail::locator_for<Ats...>;

  public:
    using name_t = Name;
    using field_id = field_t<Name, T, T{}, match::always_t, Ats...>;
    using value_type = T;
    constexpr static auto default_value = DefaultValue;
    constexpr static auto max_dword_extent = detail::max_dword_extent<Ats...>;

    template <typename DataType>
    [[nodiscard]] constexpr static auto extract(DataType const &data)
        -> value_type {
        static_assert(
            std::same_as<typename DataType::value_type, std::uint32_t>);
        return locator_t::template extract<spec_t>(
            std::span{data.begin(), data.end()});
    }

    template <typename DataType>
    constexpr static void insert(DataType &data, value_type const &value) {
        static_assert(
            std::same_as<typename DataType::value_type, std::uint32_t>);
        locator_t::template insert<spec_t>(std::span{data.begin(), data.end()},
                                           value);
    }

    template <typename DataType> constexpr static auto fits_inside() -> bool {
        static_assert(
            std::same_as<typename DataType::value_type, std::uint32_t>);
        return locator_t::template fits_inside<DataType::max_num_dwords>();
    }

    using matcher_t = M;

    template <T expected_value>
    constexpr static msg::equal_to_t<field_t, T, expected_value> equal_to{};

    constexpr static msg::equal_to_t<field_t, T, DefaultValue> match_default{};

    template <T... expected_values>
    constexpr static msg::in_t<field_t, T, expected_values...> in{};

    template <T expected_value>
    constexpr static msg::greater_than_t<field_t, T, expected_value>
        greater_than{};

    template <T expected_value>
    constexpr static msg::greater_than_or_equal_to_t<field_t, T, expected_value>
        greater_than_or_equal_to{};

    template <T expected_value>
    constexpr static msg::less_than_t<field_t, T, expected_value> less_than{};

    template <T expected_value>
    constexpr static msg::less_than_or_equal_to_t<field_t, T, expected_value>
        less_than_or_equal_to{};

    template <T NewDefaultValue>
    using WithDefault = field_t<Name, T, NewDefaultValue, M, Ats...>;

    template <typename NewMatcher>
    using WithMatch = field_t<Name, T, DefaultValue, NewMatcher, Ats...>;

    template <T NewRequiredValue>
    using WithRequired =
        field_t<Name, T, NewRequiredValue,
                msg::equal_to_t<field_t, T, NewRequiredValue>, Ats...>;

    template <T... PotentialValues>
    using WithIn = field_t<Name, T, T{},
                           msg::in_t<field_t, T, PotentialValues...>, Ats...>;

    template <T NewGreaterValue>
    using WithGreaterThan =
        field_t<Name, T, NewGreaterValue,
                msg::greater_than_t<field_t, T, NewGreaterValue>, Ats...>;

    template <T NewGreaterValue>
    using WithGreaterThanOrEqualTo =
        field_t<Name, T, NewGreaterValue,
                msg::greater_than_or_equal_to_t<field_t, T, NewGreaterValue>,
                Ats...>;

    template <T NewLesserValue>
    using WithLessThan =
        field_t<Name, T, NewLesserValue,
                msg::less_than_or_equal_to_t<field_t, T, NewLesserValue>,
                Ats...>;

    template <T NewLesserValue>
    using WithLessThanOrEqualTo =
        field_t<Name, T, NewLesserValue,
                msg::less_than_or_equal_to_t<field_t, T, NewLesserValue>,
                Ats...>;

    [[nodiscard]] constexpr static auto describe(value_type v) {
        return format("{}: 0x{:x}"_sc, name_t{}, static_cast<std::uint32_t>(v));
    }
};

template <stdx::ct_string Name, typename T = std::uint32_t,
          T DefaultValue = T{}, match::matcher M = match::always_t>
struct field {
    template <at... Ats>
    using located =
        field_t<decltype(stdx::ct_string_to_type<Name, sc::string_constant>()),
                T, DefaultValue, M, Ats...>;
};
} // namespace msg
