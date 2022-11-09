#pragma once

#include <cib/tuple.hpp>
#include <sc/string_constant.hpp>

#include <cstdint>
#include <type_traits>

namespace msg {
template <typename FieldType, typename T, T expected_value> struct equal_to_t {
    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return expected_value == msg.template get<FieldType>();
    }

    [[nodiscard]] constexpr auto describe() const {
        if constexpr (std::is_integral_v<T>) {
            return format("{} == 0x{:x}"_sc, FieldType::name,
                          sc::int_<static_cast<std::uint32_t>(expected_value)>);
        } else {
            return format("{} == {} (0x{:x})"_sc, FieldType::name,
                          sc::enum_<expected_value>,
                          sc::int_<static_cast<std::uint32_t>(expected_value)>);
        }
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        if constexpr (std::is_integral_v<T>) {
            return format(
                "{} (0x{:x}) == 0x{:x}"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(expected_value)>);
        } else {
            return format(
                "{} (0x{:x}) == {} (0x{:x})"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::enum_<expected_value>,
                sc::int_<static_cast<std::uint32_t>(expected_value)>);
        }
    }
};

template <typename FieldType, typename T, T... expected_values> struct in_t {
    static constexpr auto expected_values_tuple =
        cib::make_tuple(expected_values...);

    static constexpr auto expected_value_strings_tuple =
        cib::transform(expected_values_tuple, [](auto v) {
            if constexpr (std::is_integral_v<T>) {
                return format("0x{:x}"_sc, v);
            } else {
                return format("{} (0x{:x})"_sc, sc::enum_<v.value>,
                              sc::int_<static_cast<std::uint32_t>(v.value)>);
            }
        });

    static constexpr auto expected_values_string =
        expected_value_strings_tuple.fold_right(
            [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        auto const actual_value = msg.template get<FieldType>();

        return expected_values_tuple.fold_right(
            false, [&](auto expected_value, bool match) {
                return match || (actual_value == expected_value);
            });
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} in [{}]"_sc, FieldType::name, expected_values_string);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) in [{}]"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      expected_values_string);
    }
};

template <typename FieldType, typename T, T expected_value>
struct greater_than_t {
    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() > expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} > 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) > 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value>
struct greater_than_or_equal_to_t {
    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() >= expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} >= 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) >= 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value> struct less_than_t {
    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() < expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} < 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) < 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value>
struct less_than_or_equal_to_t {
    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() <= expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} <= 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) <= 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};
} // namespace msg
