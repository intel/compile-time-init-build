#pragma once
#include <stdx/concepts.hpp>

#include <utility>

struct [[nodiscard]] test_critical_section {
    test_critical_section() { ++count; }
    ~test_critical_section() { ++count; }
    static inline int count = 0;
};

struct test_conc_policy {
    template <typename = void, stdx::invocable F, stdx::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, Pred &&...pred)
        -> decltype(std::forward<F>(f)()) {
        while (true) {
            [[maybe_unused]] test_critical_section cs{};
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
        }
    }
};
