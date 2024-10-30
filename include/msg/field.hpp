#pragma once

#include <match/constant.hpp>
#include <match/ops.hpp>
#include <msg/field_matchers.hpp>
#include <sc/format.hpp>

#include <stdx/bit.hpp>
#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/iterator.hpp>
#include <stdx/ranges.hpp>
#include <stdx/type_traits.hpp>

#include <algorithm>
#include <climits>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <span>
#include <type_traits>

namespace msg {
template <typename T>
concept field_spec = std::unsigned_integral<decltype(T::size)> and
                     std::is_trivially_copyable_v<typename T::type> and
                     requires { typename T::name_t; };

template <typename T>
concept bits_extractor =
    std::unsigned_integral<decltype(T::size)> and
    requires(std::span<std::uint32_t const> extract_buffer) {
        {
            T::template extract<std::uint32_t>(extract_buffer)
        } -> std::same_as<std::uint32_t>;
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

template <typename T, typename Spec>
concept field_inserter_for = requires(std::span<std::uint32_t> insert_buffer,
                                      typename Spec::type value) {
    { T::template insert<Spec>(insert_buffer, value) } -> std::same_as<void>;
};

template <typename T, typename Spec>
concept field_locator_for =
    field_extractor_for<T, Spec> and field_inserter_for<T, Spec>;

template <typename T>
concept field_location = requires(T const &t) {
    { t.index() } -> std::same_as<std::uint32_t>;
    { t.lsb() } -> std::same_as<std::uint32_t>;
    { t.size() } -> std::same_as<std::uint32_t>;
    { t.valid() } -> std::same_as<bool>;
    { t.sort_key() } -> std::same_as<std::uint32_t>;
};

namespace detail {
template <typename Name, typename T, std::uint32_t BitSize>
struct field_spec_t {
    using type = T;
    using name_t = Name;

    constexpr static name_t name{};
    constexpr static auto size = BitSize;
};

template <std::uint32_t Index, std::uint32_t BitSize, std::uint32_t Lsb>
struct bits_locator_t {
    constexpr static auto size = BitSize;

    template <std::unsigned_integral T>
    [[nodiscard]] constexpr static auto fold(T value) -> T {
        if constexpr (BitSize == stdx::bit_size<T>()) {
            return {};
        } else {
            return static_cast<T>(value >> BitSize);
        }
    }

    template <std::unsigned_integral T>
    [[nodiscard]] constexpr static auto unfold(T value) -> T {
        if constexpr (BitSize == stdx::bit_size<T>()) {
            return {};
        } else {
            return static_cast<T>(value << BitSize);
        }
    }

    template <std::unsigned_integral E, typename R>
    [[nodiscard]] constexpr static auto extract(R &&r) -> E {
        using elem_t = typename std::remove_cvref_t<R>::value_type;
        using T = std::make_unsigned_t<decltype(elem_t{} & E{})>;
        constexpr auto Msb = Lsb + BitSize - 1u;

        constexpr auto BaseIndex =
            Index * sizeof(std::uint32_t) / sizeof(elem_t);

        constexpr auto elem_size = stdx::bit_size<elem_t>();
        constexpr auto max_idx = BaseIndex + Msb / elem_size;
        constexpr auto min_idx = BaseIndex + Lsb / elem_size;

        constexpr auto f =
            []<auto CurrentMsb, typename Rng>([[maybe_unused]] auto recurse,
                                              Rng &&rng, T value) -> T {
            constexpr auto current_idx = BaseIndex + CurrentMsb / elem_size;
            if constexpr (current_idx == max_idx and current_idx == min_idx) {
                constexpr auto mask =
                    stdx::bit_mask<T, CurrentMsb % elem_size>();
                constexpr auto shift = Lsb % elem_size;
                return (std::forward<Rng>(rng)[current_idx] & mask) >> shift;
            } else if constexpr (current_idx == min_idx) {
                constexpr auto shift = Lsb % elem_size;
                value <<= (elem_size - shift);
                return value | (std::forward<Rng>(rng)[current_idx] >> shift);
            } else if constexpr (current_idx == max_idx) {
                constexpr auto mask =
                    stdx::bit_mask<T, CurrentMsb % elem_size>();
                constexpr auto NewMsb =
                    (CurrentMsb / elem_size) * elem_size - 1u;
                MUSTTAIL return recurse.template operator()<NewMsb>(
                    recurse, std::forward<Rng>(rng),
                    std::forward<Rng>(rng)[current_idx] & mask);
            } else {
                value <<= elem_size;
                value |= std::forward<Rng>(rng)[current_idx];
                constexpr auto NewMsb = CurrentMsb - elem_size;
                MUSTTAIL return recurse.template operator()<NewMsb>(
                    recurse, std::forward<Rng>(rng), value);
            }
        };
        return static_cast<E>(
            f.template operator()<Msb>(f, std::forward<R>(r), T{}));
    }

    template <std::unsigned_integral E, typename R>
    constexpr static auto insert(R &&r, E e) -> void {
        using elem_t = typename std::remove_cvref_t<R>::value_type;
        using T = std::make_unsigned_t<decltype(E{} >> 1u)>;
        constexpr auto Msb = Lsb + BitSize - 1u;

        constexpr auto BaseIndex =
            Index * sizeof(std::uint32_t) / sizeof(elem_t);

        constexpr auto elem_size = stdx::bit_size<elem_t>();
        [[maybe_unused]] constexpr auto min_idx = BaseIndex + Lsb / elem_size;
        [[maybe_unused]] constexpr auto max_idx = BaseIndex + Msb / elem_size;

        constexpr auto f =
            []<auto CurrentLsb, typename Rng>([[maybe_unused]] auto recurse,
                                              Rng &&rng, T value) -> void {
            constexpr auto current_idx = BaseIndex + CurrentLsb / elem_size;

            if constexpr (current_idx == max_idx) {
                constexpr auto lsb = CurrentLsb % elem_size;
                constexpr auto msb = Msb % elem_size;
                constexpr auto leftover_mask =
                    ~stdx::bit_mask<elem_t, msb, lsb>();

                auto &elem = std::forward<Rng>(rng)[current_idx];
                elem &= leftover_mask;
                elem |= static_cast<elem_t>(value << lsb);
            } else if constexpr (current_idx == min_idx) {
                constexpr auto lsb = CurrentLsb % elem_size;
                constexpr auto numbits = elem_size - lsb;
                constexpr auto value_mask =
                    stdx::bit_mask<elem_t, numbits - 1>();
                constexpr auto leftover_mask = ~(value_mask << lsb);

                auto &elem = std::forward<Rng>(rng)[current_idx];
                elem &= leftover_mask;
                elem |= static_cast<elem_t>((value & value_mask) << lsb);

                constexpr auto NewLsb = CurrentLsb + numbits;
                MUSTTAIL return recurse.template operator()<NewLsb>(
                    recurse, std::forward<Rng>(rng), value >> numbits);
            } else {
                constexpr auto value_mask = stdx::bit_mask<elem_t>();

                auto &elem = std::forward<Rng>(rng)[current_idx];
                elem = value & value_mask;

                constexpr auto NewLsb = CurrentLsb + elem_size;
                MUSTTAIL return recurse.template operator()<NewLsb>(
                    recurse, std::forward<Rng>(rng), value >> elem_size);
            }
        };
        f.template operator()<Lsb>(f, std::forward<R>(r), T{e});
    }

    template <std::uint32_t NumBits>
    constexpr static auto fits_inside() -> bool {
        constexpr auto Msb = Lsb + BitSize - 1;
        return Index * 32 + Msb <= NumBits;
    }

    template <typename T> constexpr static auto extent_in() -> std::size_t {
        constexpr auto msb = Lsb + BitSize - 1;
        constexpr auto msb_extent = (msb + CHAR_BIT - 1) / CHAR_BIT;
        constexpr auto base_extent = Index * sizeof(T);
        constexpr auto extent = base_extent + msb_extent;
        return (extent + sizeof(T) - 1) / sizeof(T);
    }
};

template <typename T> CONSTEVAL auto select_integral_type() {
    if constexpr (sizeof(T) <= sizeof(std::uint8_t)) {
        return std::uint8_t{};
    } else if constexpr (sizeof(T) <= sizeof(std::uint16_t)) {
        return std::uint16_t{};
    } else if constexpr (sizeof(T) <= sizeof(std::uint32_t)) {
        return std::uint32_t{};
    } else {
        static_assert(sizeof(T) <= sizeof(std::uint64_t),
                      "T is too big to be covered by an integral type!");
        return std::uint64_t{};
    }
}
template <typename T>
using integral_type_for = decltype(select_integral_type<T>());

template <bits_locator... BLs> struct field_locator_t {
    template <field_spec Spec, stdx::range R>
    [[nodiscard]] constexpr static auto extract(R &&r) -> typename Spec::type {
        using raw_t = integral_type_for<typename Spec::type>;
        auto raw = raw_t{};
        auto const extract_bits = [&]<bits_locator B>() {
            raw = B::unfold(raw);
            raw |= B::template extract<raw_t>(std::forward<R>(r));
        };
        (..., extract_bits.template operator()<BLs>());
        return stdx::bit_cast<typename Spec::type>(raw);
    }

    template <field_spec Spec, stdx::range R>
    constexpr static auto insert(R &&r,
                                 typename Spec::type const &value) -> void {
        using raw_t = integral_type_for<typename Spec::type>;
        auto raw = stdx::bit_cast<raw_t>(value);
        auto const insert_bits = [&]<bits_locator B>() {
            B::insert(
                std::forward<R>(r),
                static_cast<raw_t>(raw & stdx::bit_mask<raw_t, B::size - 1>()));
            raw = B::fold(raw);
        };

        [[maybe_unused]] int dummy{};
        (void)(dummy = ... = (insert_bits.template operator()<BLs>(), 0));
    }

    template <std::uint32_t NumBits>
    constexpr static auto fits_inside() -> bool {
        return (... and BLs::template fits_inside<NumBits>());
    }

    template <typename T> constexpr static auto extent_in() -> std::size_t {
        return std::max({std::size_t{}, BLs::template extent_in<T>()...});
    }

    constexpr static auto size = (std::size_t{} + ... + BLs::size);
};
} // namespace detail

enum struct byte_index_t : std::uint32_t {};
enum struct dword_index_t : std::uint32_t {};
enum struct msb_t : std::uint32_t {};
enum struct lsb_t : std::uint32_t {};

inline namespace literals {
// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_bi(unsigned long long int v) -> byte_index_t {
    return static_cast<byte_index_t>(v);
}
CONSTEVAL auto operator""_dwi(unsigned long long int v) -> dword_index_t {
    return static_cast<dword_index_t>(v);
}
CONSTEVAL auto operator""_dw(unsigned long long int v) -> dword_index_t {
    return static_cast<dword_index_t>(v);
}
// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_lsb(unsigned long long int v) -> lsb_t {
    return static_cast<lsb_t>(v);
}
// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_msb(unsigned long long int v) -> msb_t {
    return static_cast<msb_t>(v);
}
} // namespace literals

template <typename...> struct at;

template <> struct at<dword_index_t, msb_t, lsb_t> {
    dword_index_t index_{};
    msb_t msb_{};
    lsb_t lsb_{};

    [[nodiscard]] constexpr auto
    index() const -> std::underlying_type_t<dword_index_t> {
        return stdx::to_underlying(index_);
    }
    [[nodiscard]] constexpr auto lsb() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(lsb_);
    }
    [[nodiscard]] constexpr auto size() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(msb_) - lsb() + 1;
    }
    [[nodiscard]] constexpr auto valid() const -> bool {
        return size() <= 64 and
               stdx::to_underlying(msb_) >= stdx::to_underlying(lsb_);
    }
    [[nodiscard]] constexpr auto
    sort_key() const -> std::underlying_type_t<lsb_t> {
        return index() * 32u + stdx::to_underlying(lsb_);
    }
};

template <> struct at<byte_index_t, msb_t, lsb_t> {
    byte_index_t index_{};
    msb_t msb_{};
    lsb_t lsb_{};

    [[nodiscard]] constexpr auto
    index() const -> std::underlying_type_t<byte_index_t> {
        return stdx::to_underlying(index_) / 4;
    }
    [[nodiscard]] constexpr auto lsb() const -> std::underlying_type_t<lsb_t> {
        return (stdx::to_underlying(index_) * 8) % 32 +
               stdx::to_underlying(lsb_);
    }
    [[nodiscard]] constexpr auto size() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(msb_) - stdx::to_underlying(lsb_) + 1;
    }
    [[nodiscard]] constexpr auto valid() const -> bool {
        return size() <= 8 and
               stdx::to_underlying(msb_) >= stdx::to_underlying(lsb_);
    }
    [[nodiscard]] constexpr auto
    sort_key() const -> std::underlying_type_t<lsb_t> {
        return index() * 32u + stdx::to_underlying(lsb_);
    }
};

template <> struct at<msb_t, lsb_t> {
    msb_t msb_{};
    lsb_t lsb_{};

    [[nodiscard]] constexpr auto
    index() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(lsb_) / 32u;
    }
    [[nodiscard]] constexpr auto lsb() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(lsb_) % 32u;
    }
    [[nodiscard]] constexpr auto size() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(msb_) - stdx::to_underlying(lsb_) + 1;
    }
    [[nodiscard]] constexpr auto valid() const -> bool {
        return size() <= 64 and
               stdx::to_underlying(msb_) >= stdx::to_underlying(lsb_);
    }
    [[nodiscard]] constexpr auto
    sort_key() const -> std::underlying_type_t<lsb_t> {
        return stdx::to_underlying(lsb_);
    }
};

template <typename... Ts> at(Ts...) -> at<Ts...>;

namespace detail {
template <auto... Ats>
    requires(... and field_location<decltype(Ats)>)
using locator_for =
    field_locator_t<bits_locator_t<Ats.index(), Ats.size(), Ats.lsb()>...>;

template <at... Ats> constexpr inline auto field_size = (0u + ... + Ats.size());

template <typename T, T V = T{}> struct with_default {
    constexpr static auto default_value = V;
    template <typename F> using default_matcher_t = msg::equal_to_t<F, V>;
    using is_mutable_t = void;
    template <T X> constexpr static auto is_compatible_value = true;
};

template <typename T, T V = T{}> struct with_const_default {
    constexpr static auto default_value = V;
    template <typename F> using default_matcher_t = msg::equal_to_t<F, V>;
    template <T X> constexpr static auto is_compatible_value = X == V;
};

struct without_default {
    template <typename F> using default_matcher_t = match::never_t;
    using is_mutable_t = void;
    template <auto X> constexpr static auto is_compatible_value = true;
};

template <typename T>
concept has_default_value = requires { T::default_value; };

template <typename T>
using has_default_value_t = std::bool_constant<has_default_value<T>>;

template <typename T>
concept is_mutable_value = requires { typename T::is_mutable_t; };

template <auto V, typename D>
concept compatible_with_default = D::template is_compatible_value<V>;

template <typename Name, typename T = std::uint32_t, auto... Ats>
struct field_id_t {};

template <typename Name, typename T = std::uint32_t,
          typename Default = detail::with_default<T>,
          match::matcher M = match::always_t, auto... Ats>
    requires std::is_trivially_copyable_v<T> and
                 (... and field_location<decltype(Ats)>)
class field_t : public field_spec_t<Name, T, detail::field_size<Ats...>>,
                public detail::locator_for<Ats...>,
                public Default {
    using spec_t = field_spec_t<Name, T, detail::field_size<Ats...>>;
    using locator_t = detail::locator_for<Ats...>;

    [[nodiscard]] constexpr static auto valid_location(auto const &at) -> bool {
        return at.size() <= 64 and at.msb() >= at.lsb();
    }
    static_assert((... and Ats.valid()),
                  "Individual field location size cannot exceed 64 bits!");
    static_assert(stdx::bit_size<T>() >= (0u + ... + Ats.size()),
                  "Field size is smaller than sum of locations!");

  public:
    using name_t = Name;
    using field_id = field_id_t<Name, T, Ats...>;
    using value_type = T;
    using matcher_t = M;
    constexpr static auto sort_key = std::min({Ats.sort_key()...});

    template <stdx::range R>
    [[nodiscard]] constexpr static auto extract(R &&r) -> value_type {
        return locator_t::template extract<spec_t>(std::forward<R>(r));
    }

    template <stdx::range R>
    constexpr static void insert(R &&r, value_type const &value) {
        static_assert(is_mutable_value<field_t>,
                      "Can't change a field with a required value!");
        locator_t::template insert<spec_t>(std::forward<R>(r), value);
    }

    template <stdx::range R> constexpr static void insert_default(R &&r) {
        if constexpr (has_default_value<Default>) {
            locator_t::template insert<spec_t>(std::forward<R>(r),
                                               Default::default_value);
        }
    }

    template <typename DataType> constexpr static auto fits_inside() -> bool {
        constexpr auto bits_capacity =
            stdx::bit_size<typename DataType::value_type>() *
            stdx::ct_capacity_v<DataType>;
        return locator_t::template fits_inside<bits_capacity>();
    }

    template <typename U> constexpr static auto extent_in() -> std::size_t {
        return locator_t::template extent_in<U>();
    }

    [[nodiscard]] constexpr static auto describe(value_type v) {
        return format("{}: 0x{:x}"_sc, spec_t::name, v);
    }

    constexpr static auto can_hold(value_type v) -> bool {
        return locator_t::size >= static_cast<std::size_t>(stdx::bit_width(v));
    }

    // ======================================================================
    // default construction values
    template <T V>
    using with_default =
        field_t<Name, T, detail::with_default<T, V>, M, Ats...>;

    template <T V>
    using with_const_default =
        field_t<Name, T, detail::with_const_default<T, V>, M, Ats...>;

    using without_default =
        field_t<Name, T, detail::without_default, M, Ats...>;

    // ======================================================================
    // matcher values
    template <typename NewMatcher>
    using with_matcher = field_t<Name, T, Default, NewMatcher, Ats...>;

    template <T V>
        requires compatible_with_default<V, Default>
    using with_equal_to =
        field_t<Name, T, Default, msg::equal_to_t<field_t, V>, Ats...>;

    template <T... Vs>
        requires(... or compatible_with_default<Vs, Default>)
    using with_in =
        field_t<Name, T, Default, msg::in_t<field_t, Vs...>, Ats...>;

    template <T V>
    using with_greater_than =
        field_t<Name, T, Default, msg::greater_than_t<field_t, V>, Ats...>;

    template <T V>
    using with_greater_than_or_equal_to =
        field_t<Name, T, Default, msg::greater_than_or_equal_to_t<field_t, V>,
                Ats...>;

    template <T V>
    using with_less_than =
        field_t<Name, T, Default, msg::less_than_or_equal_to_t<field_t, V>,
                Ats...>;

    template <T V>
    using with_less_than_or_equal_to =
        field_t<Name, T, Default, msg::less_than_or_equal_to_t<field_t, V>,
                Ats...>;

    // ======================================================================
    // "const value" for construction and matching
    template <T V>
    using with_required = field_t<Name, T, detail::with_const_default<T, V>,
                                  msg::equal_to_t<field_t, V>, Ats...>;

    // ======================================================================
    // legacy aliases
    template <typename X> using WithMatch = with_matcher<X>;
    template <T... Vs> using WithIn = with_in<Vs...>;
    template <T V> using WithGreaterThan = with_greater_than<V>;
    template <T V>
    using WithGreaterThanOrEqualTo = with_greater_than_or_equal_to<V>;
    template <T V> using WithLessThan = with_less_than<V>;
    template <T V> using WithLessThanOrEqualTo = with_less_than_or_equal_to<V>;
    template <T V> using WithRequired = with_required<V>;
};
} // namespace detail

template <stdx::ct_string Name, typename T = std::uint32_t,
          typename Default = detail::with_default<T>,
          match::matcher M = match::always_t>
struct field {
    template <auto... Ats>
        requires(... and field_location<decltype(Ats)>)
    using located = detail::field_t<
        decltype(stdx::ct_string_to_type<Name, sc::string_constant>()), T,
        Default, M, Ats...>;
};
} // namespace msg
