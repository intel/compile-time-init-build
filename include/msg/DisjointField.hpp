#pragma once

#include <type_traits>
#include <cstdint>

#include <sc/string_constant.hpp>
#include <msg/Match.hpp>

#include <msg/FieldMatchers.hpp>


/**
 * DisjointField is used when a field contains disjoint spans of bits.
 */
template<
    typename NameTypeT,
    typename FieldsT,
    typename T = std::uint32_t,
    T DefaultValue = T{},
    typename MatchRequirementsType = match::Always<true>>
class DisjointField {
private:
    static constexpr FieldsT fields{};
    T value;

public:
    static constexpr size_t size =
        fields.fold_right(0, [](auto f, size_t totalSize){
            return totalSize + f.size;
        });

    using FieldId = DisjointField<NameTypeT, FieldsT, T>;
    using This = DisjointField<NameTypeT, FieldsT, T, DefaultValue, MatchRequirementsType>;

    using NameType = NameTypeT;


    template<typename MsgType>
    static constexpr void fitsInside(MsgType msg) {
        fields.for_each([&](auto field){
            field.fitsInside(msg);
        });
    }


    static constexpr NameType name{};
    static constexpr MatchRequirementsType matchRequirements{};


    template<T expectedValue>
    static constexpr msg::EqualTo<This, T, expectedValue> equalTo{};

    static constexpr msg::EqualTo<This, T, DefaultValue> matchDefault{};

    template<T... expectedValues>
    static constexpr msg::In<This, T, expectedValues...> in{};

    template<T expectedValue>
    static constexpr msg::GreaterThan<This, T, expectedValue> greaterThan{};

    template<T expectedValue>
    static constexpr msg::GreaterThanOrEqualTo<This, T, expectedValue> greaterThanOrEqualTo{};

    template<T expectedValue>
    static constexpr msg::LessThan<This, T, expectedValue> lessThan{};

    template<T expectedValue>
    static constexpr msg::LessThanOrEqualTo<This, T, expectedValue> lessThanOrEqualTo{};


    template<T NewDefaultValue>
    using WithDefault = DisjointField<NameTypeT, FieldsT, T, NewDefaultValue>;

    template<T NewRequiredValue>
    using WithRequired = DisjointField<NameTypeT, FieldsT, T, NewRequiredValue, msg::EqualTo<This, T, NewRequiredValue>>;

    template<T... PotentialValues>
    using WithIn = DisjointField<NameTypeT, FieldsT, T, T{}, msg::In<This, T, PotentialValues...>>;

    template<typename NewRequiredMatcher>
    using WithMatch = DisjointField<NameTypeT, FieldsT, T, DefaultValue, NewRequiredMatcher>;


    constexpr DisjointField(T const & newValue)
        : value{newValue}
    {
        // pass
    }

    constexpr DisjointField()
        : value{DefaultValue}
    {
        // pass
    }

    template<typename DataType>
    [[nodiscard]] static constexpr T extract(DataType const & data) {
        auto const raw =
            fields.fold_left(static_cast<std::uint64_t>(0), [&](std::uint64_t extracted, auto f){
                return (extracted << f.size) | f.extract(data);
            });

        return static_cast<T>(raw);
    }

    template<typename DataType>
    constexpr void insert(DataType & data) const {
        fields.fold_right(static_cast<uint64_t>(value), [&](auto fieldPrototype, uint64_t remaining){
            using FieldType = decltype(fieldPrototype);
            using ValueType = typename FieldType::ValueType;
            decltype(fieldPrototype) const f{static_cast<ValueType>(remaining)};
            f.insert(data);
            return remaining >> f.size;
        });
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{}: 0x{:x}"_sc, name, static_cast<std::uint32_t>(value));
    }
};
