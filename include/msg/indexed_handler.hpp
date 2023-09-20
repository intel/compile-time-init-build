#pragma once

#include <msg/detail/indexed_handler_common.hpp>

#include <stdx/compiler.hpp>

namespace msg {

template <typename... IndicesT> struct indices : IndicesT... {
    CONSTEVAL explicit indices(IndicesT... index_args)
        : IndicesT{index_args}... {}

    constexpr auto operator()(auto const &data) const {
        return (this->IndicesT::operator()(data) & ...);
    }
};

} // namespace msg
