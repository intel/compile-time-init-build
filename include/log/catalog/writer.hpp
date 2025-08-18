#pragma once

#include <log/catalog/mipi_builder.hpp>

#include <stdx/compiler.hpp>
#include <stdx/span.hpp>

#include <cstdint>
#include <utility>

namespace logging::binary {
template <typename T>
concept writer_like =
    requires(T &t, stdx::span<std::uint32_t const, 1> data) { t(data); };

[[maybe_unused]] constexpr inline struct get_writer_t {
    template <typename T>
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_writer_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }
} get_writer;
} // namespace logging::binary
