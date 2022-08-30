#pragma once


#include <type_traits>
#include <sc/string_constant.hpp>

#include <cstdint>


namespace msg {
    template<typename FieldType, typename T, T expectedValue>
    struct EqualTo {
        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            return expectedValue == msg.template get<FieldType>();
        }

        [[nodiscard]] constexpr auto describe() const {
            if constexpr (std::is_integral_v<T>) {
                return format("{} == 0x{:x}"_sc,
                    FieldType::name,
                    sc::int_<static_cast<std::uint32_t>(expectedValue)>);
            } else {
                return format("{} == {} (0x{:x})"_sc,
                    FieldType::name,
                    sc::enum_<expectedValue>,
                    sc::int_<static_cast<std::uint32_t>(expectedValue)>);
            }
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            if constexpr (std::is_integral_v<T>) {
                return format("{} (0x{:x}) == 0x{:x}"_sc,
                    FieldType::name,
                    static_cast<std::uint32_t>(msg.template get<FieldType>()),
                    sc::int_<static_cast<std::uint32_t>(expectedValue)>);
            } else {
                return format("{} (0x{:x}) == {} (0x{:x})"_sc,
                    FieldType::name,
                    static_cast<std::uint32_t>(msg.template get<FieldType>()),
                    sc::enum_<expectedValue>,
                    sc::int_<static_cast<std::uint32_t>(expectedValue)>);
            }
        }
    };

    template<typename FieldType, typename T, T... expectedValues>
    struct In {
        static constexpr auto expectedValuesTuple = cib::make_tuple(expectedValues...);

        static constexpr auto expectedValueStringsTuple =
            cib::transform(expectedValuesTuple, [](auto v){
                if constexpr (std::is_integral_v<T>) {
                    return format("0x{:x}"_sc, v);
                } else {
                    return format("{} (0x{:x})"_sc,
                        sc::enum_<v.value>,
                        sc::int_<static_cast<std::uint32_t>(v.value)>);
                }
            });

        static constexpr auto expectedValuesString =
            expectedValueStringsTuple.fold_right([](auto lhs, auto rhs){
                return lhs + ", "_sc + rhs;
            });

        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            auto const actualValue = msg.template get<FieldType>();

            return expectedValuesTuple.fold_right(false, [&](auto expectedValue, bool match){
                return match || (actualValue == expectedValue);
            });
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("{} in [{}]"_sc, FieldType::name, expectedValuesString);
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            return format("{} (0x{:x}) in [{}]"_sc,
                FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                expectedValuesString);
        }
    };

    template<typename FieldType, typename T, T expectedValue>
    struct GreaterThan {
        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            return msg.template get<FieldType>() > expectedValue;
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("{} > 0x{:x}"_sc,
                FieldType::name,
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            return format("{} (0x{:x}) > 0x{:x}"_sc,
                FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }
    };

    template<typename FieldType, typename T, T expectedValue>
    struct GreaterThanOrEqualTo {
        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            return msg.template get<FieldType>() >= expectedValue;
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("{} >= 0x{:x}"_sc,
                FieldType::name,
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            return format("{} (0x{:x}) >= 0x{:x}"_sc,
                FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }
    };

    template<typename FieldType, typename T, T expectedValue>
    struct LessThan {
        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            return msg.template get<FieldType>() < expectedValue;
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("{} < 0x{:x}"_sc,
                FieldType::name,
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            return format("{} (0x{:x}) < 0x{:x}"_sc,
                FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }
    };

    template<typename FieldType, typename T, T expectedValue>
    struct LessThanOrEqualTo {
        template<typename MsgType>
        [[nodiscard]] constexpr bool operator()(MsgType const & msg) const {
            return msg.template get<FieldType>() <= expectedValue;
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("{} <= 0x{:x}"_sc,
                FieldType::name,
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }

        template<typename MsgType>
        [[nodiscard]] constexpr auto describeMatch(MsgType const & msg) const {
            return format("{} (0x{:x}) <= 0x{:x}"_sc,
                FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(expectedValue)>);
        }
    };
}
