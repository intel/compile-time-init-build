#pragma once

#include <match/concepts.hpp>
#include <match/sum_of_products.hpp>

#include <stdx/concepts.hpp>

#include <type_traits>
#include <utility>

namespace msg {
template <typename Name, match::matcher M, stdx::callable F>
struct indexed_callback_t {
    constexpr static Name name{};
    M matcher;
    F callable;
};

constexpr auto indexed_callback =
    []<typename Name, match::matcher M, stdx::callable F>(Name, M &&m, F &&f) {
        auto sop = match::sum_of_products(std::forward<M>(m));
        return indexed_callback_t<Name, decltype(sop), std::remove_cvref_t<F>>{
            std::move(sop), std::forward<F>(f)};
    };
} // namespace msg
