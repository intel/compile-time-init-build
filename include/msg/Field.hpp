#pragma once


#include <type_traits>
#include <cstdint>

#include <sc/string_constant.hpp>
#include <msg/Match.hpp>

#include <msg/FieldMatchers.hpp>


// TODO: handle field types that are not integral types (like QosScaledValue)
// TODO: add support for fields with non-contiguous bits
/**
 * Field class used to specify field lengths and manipulate desired field from provided data
 * @tparam MsbT  most significant bit position for the field
 * @tparam LsbT  least significant bit position for the field
 */
template<
    typename NameTypeT,
    std::uint32_t DWordIndexT,
    std::uint32_t MsbT,
    std::uint32_t LsbT,
    typename T = std::uint32_t,
    T DefaultValue = T{},
    typename MatchRequirementsType = match::Always<true>>
class Field {
private:
    T value;

public:
    static_assert(LsbT <= MsbT, "msb needs to be lower than or equal to lsb");
    static_assert(LsbT <= 31, "lsb needs to be lower than or equal to 31");

    static constexpr size_t size = (MsbT - LsbT) + 1;
    static_assert(size <= 64, "Field must be 64 bits or smaller");

    using FieldId = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T>;
    using This = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, DefaultValue, MatchRequirementsType>;
    using ValueType = T;

    template<typename MsgType>
    static constexpr void fitsInside(MsgType) {
        static_assert(DWordIndex < MsgType::NumDWords);
    }

    using NameType = NameTypeT;
    static constexpr auto DWordIndex = DWordIndexT;

    static constexpr NameType name{};
    static constexpr uint64_t bitMask = []{
        if constexpr (size == 64) {
            return 0xFFFFFFFFFFFFFFFFUL;
        } else {
            return (static_cast<uint64_t>(1) << static_cast<uint64_t>(size)) - static_cast<uint64_t>(1);
        }
    }();

    static constexpr uint64_t fieldMask = bitMask << LsbT;

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

    template<T NewGreaterValue>
    using WithGreaterThan = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, NewGreaterValue, msg::GreaterThan<This, T, NewGreaterValue>>;

    template<T NewDefaultValue>
    using WithDefault = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, NewDefaultValue>;

    template<T NewRequiredValue>
    using WithRequired = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, NewRequiredValue, msg::EqualTo<This, T, NewRequiredValue>>;

    template<T... PotentialValues>
    using WithIn = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, T{}, msg::In<This, T, PotentialValues...>>;

    template<typename NewRequiredMatcher>
    using WithMatch = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, DefaultValue, NewRequiredMatcher>;

    template<T NewGreaterValue>
    using WithGreaterThanOrEqualTo = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, NewGreaterValue, msg::GreaterThanOrEqualTo<This, T, NewGreaterValue>>;

    template<T NewLesserValue>
    using WithLessThanOrEqualTo = Field<NameTypeT, DWordIndexT, MsbT, LsbT, T, NewLesserValue, msg::LessThanOrEqualTo<This, T, NewLesserValue>>;

    constexpr Field(T const & newValue)
        : value{newValue}
    {
        // pass
    }

    constexpr Field()
        : value{DefaultValue}
    {
        // pass
    }

    template<typename DataType>
    [[nodiscard]] static constexpr T extract(DataType const & data) {
        std::uint32_t const lower =
            data[DWordIndex] >> LsbT;

        std::uint64_t const mid = [&]{
            if constexpr (MsbT < 32) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 1]) << (32 - LsbT);
            }
        }();

        std::uint64_t const upper = [&]{
            if constexpr (MsbT < 64) {
                return 0;
            } else {
                return static_cast<std::uint64_t>(data[DWordIndex + 2]) << (64 - LsbT);
            }
        }();

        return static_cast<T>((upper | mid | lower) & bitMask);
    }

    template<typename DataType>
    constexpr void insert(DataType & data) const {
        data[DWordIndex] =
            ((static_cast<uint32_t>(value) << LsbT) & fieldMask) | (data[DWordIndex] & ~fieldMask);

        if constexpr (MsbT >= 32) {
            constexpr uint32_t fieldMaskDWord1 =
                static_cast<std::uint32_t>(bitMask >> (32 - LsbT));

            data[DWordIndex + 1] =
                ((static_cast<uint64_t>(value) >> (32 - LsbT)) & fieldMaskDWord1) | (data[DWordIndex + 1] & ~fieldMaskDWord1);
        }

        if constexpr (MsbT >= 64) {
            constexpr uint32_t fieldMaskDWord2 =
                static_cast<std::uint32_t>(bitMask >> (64 - LsbT));

            data[DWordIndex + 2] =
                ((static_cast<uint64_t>(value) >> (64 - LsbT)) & fieldMaskDWord2) | (data[DWordIndex + 2] & ~fieldMaskDWord2);
        }
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{}: 0x{:x}"_sc, name, static_cast<std::uint32_t>(value));
    }
};
