#pragma once

#include <msg/detail/indexed_handler_common.hpp>

#include <stdx/compiler.hpp>

namespace msg {

template <typename... Indices> struct indices : Indices... {
    CONSTEVAL explicit indices(Indices... index_args)
        : Indices{index_args}... {}

    constexpr auto operator()(auto const &data) const {
        return (this->Indices::operator()(data) & ...);
    }
};

} // namespace msg
