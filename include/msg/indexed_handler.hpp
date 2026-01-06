#pragma once

#include <msg/detail/indexed_handler_common.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>

#include <cstdint>

namespace msg {
template <typename... Indices> struct indices : Indices... {
    CONSTEVAL explicit indices(Indices... index_args)
        : Indices{index_args}... {}

    constexpr auto operator()(auto const &data) const {
        return (... & this->Indices::operator()(data));
    }
};

template <> struct indices<> {
    CONSTEVAL explicit indices() = default;

    constexpr auto operator()(auto const &) const
        -> stdx::bitset<0, std::uint32_t> {
        return {};
    }
};
} // namespace msg
