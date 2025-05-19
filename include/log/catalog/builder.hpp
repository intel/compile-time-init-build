#pragma once

#include <log/catalog/mipi_builder.hpp>

#include <stdx/compiler.hpp>

#include <utility>

namespace logging::binary {
[[maybe_unused]] constexpr inline struct get_builder_t {
    template <typename T>
        requires true
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_builder_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        return logging::mipi::default_builder{};
    }
} get_builder;
} // namespace logging::binary
