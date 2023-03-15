#pragma once

#include <concepts>
#include <utility>

// NOTE: this is a mock implementation of concurrency: it does nothing but
// fulfil the required interface.

namespace test {

struct concurrency_policy {
    struct [[nodiscard]] test_critical_section {
        test_critical_section() { ++count; }
        ~test_critical_section() { ++count; }
        static inline int count = 0;
    };

    template <std::invocable F, std::predicate... Pred>
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

} // namespace test
