#pragma once

#include <stdx/ct_string.hpp>

#include <cstdint>

template <typename Field> struct field_value_t {
    constexpr static auto id = Field::id;
    std::uint32_t value;
};

template <int Id, typename Reg, stdx::ct_string Name, std::uint32_t Msb,
          std::uint32_t Lsb>
struct mock_field_t {
    constexpr static auto id = Id;
    using RegisterType = Reg;
    using DataType = std::uint32_t;

    constexpr static auto get_register() -> Reg { return {}; }

    constexpr static auto get_mask() -> DataType {
        return ((1u << (Msb + 1u)) - 1u) - ((1u << Lsb) - 1u);
    }

    constexpr auto operator()(std::uint32_t value) const {
        return field_value_t<mock_field_t>{value};
    }
};

template <int Id, typename Reg, stdx::ct_string Name> struct mock_register_t {
    constexpr static auto id = Id;
    using RegisterType = Reg;
    using DataType = std::uint32_t;

    constexpr static auto get_register() -> Reg { return {}; }

    constexpr static mock_field_t<Id, Reg, "raw", 31, 0> raw{};
};

template <typename... Ops> constexpr auto apply(Ops... ops) {
    return (ops(), ...);
}
