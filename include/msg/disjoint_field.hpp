#pragma once

#include <msg/field_matchers.hpp>
#include <msg/match.hpp>
#include <sc/string_constant.hpp>

#include <cstdint>
#include <type_traits>

namespace msg {
/**
 * disjoint_field is used when a field contains disjoint spans of bits.
 */
template <typename NameTypeT, typename FieldsT, typename T = std::uint32_t,
          T DefaultValue = T{},
          typename MatchRequirementsType = match::always_t<true>>
class disjoint_field {
  private:
    static constexpr FieldsT fields{};
    T value{};

  public:
    static constexpr size_t size = fields.fold_right(
        0, [](auto f, size_t totalSize) { return totalSize + f.size; });

    using FieldId = disjoint_field<NameTypeT, FieldsT, T>;
    using This = disjoint_field<NameTypeT, FieldsT, T, DefaultValue,
                                MatchRequirementsType>;

    using NameType = NameTypeT;

    template <typename MsgType> static constexpr void fits_inside(MsgType msg) {
        cib::for_each([&](auto field) { field.fits_inside(msg); }, fields);
    }

    static constexpr NameType name{};
    static constexpr MatchRequirementsType match_requirements{};

    template <T expected_value>
    static constexpr msg::equal_to_t<This, T, expected_value> equal_to{};

    static constexpr msg::equal_to_t<This, T, DefaultValue> match_default{};

    template <T... expected_values>
    static constexpr msg::in_t<This, T, expected_values...> in{};

    template <T expected_value>
    static constexpr msg::greater_than_t<This, T, expected_value>
        greater_than{};

    template <T expected_value>
    static constexpr msg::greater_than_or_equal_to_t<This, T, expected_value>
        greater_than_or_equal_to{};

    template <T expected_value>
    static constexpr msg::less_than_t<This, T, expected_value> less_than{};

    template <T expected_value>
    static constexpr msg::less_than_or_equal_to_t<This, T, expected_value>
        less_than_or_equal_to{};

    template <T NewDefaultValue>
    using WithDefault = disjoint_field<NameTypeT, FieldsT, T, NewDefaultValue>;

    template <T NewRequiredValue>
    using WithRequired =
        disjoint_field<NameTypeT, FieldsT, T, NewRequiredValue,
                       msg::equal_to_t<This, T, NewRequiredValue>>;

    template <T... PotentialValues>
    using WithIn = disjoint_field<NameTypeT, FieldsT, T, T{},
                                  msg::in_t<This, T, PotentialValues...>>;

    template <typename NewRequiredMatcher>
    using WithMatch =
        disjoint_field<NameTypeT, FieldsT, T, DefaultValue, NewRequiredMatcher>;

    constexpr explicit disjoint_field(T const &new_value) : value{new_value} {
        // pass
    }

    constexpr disjoint_field() : value{DefaultValue} {
        // pass
    }

    template <typename DataType>
    [[nodiscard]] static constexpr auto extract(DataType const &data) -> T {
        auto const raw =
            fields.fold_left(static_cast<std::uint64_t>(0),
                             [&](std::uint64_t extracted, auto f) {
                                 return (extracted << f.size) | f.extract(data);
                             });

        return static_cast<T>(raw);
    }

    template <typename DataType> constexpr void insert(DataType &data) const {
        (void)fields.fold_right(static_cast<uint64_t>(value),
                                [&](auto fieldPrototype, uint64_t remaining) {
                                    using FieldType = decltype(fieldPrototype);
                                    using ValueType =
                                        typename FieldType::ValueType;
                                    decltype(fieldPrototype) const f{
                                        static_cast<ValueType>(remaining)};
                                    f.insert(data);
                                    return remaining >> f.size;
                                });
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{}: 0x{:x}"_sc, name, static_cast<std::uint32_t>(value));
    }
};
} // namespace msg
