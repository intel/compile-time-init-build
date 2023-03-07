#pragma once

#include <concepts>
#include <utility>

// NOTE: this is a mock implementation of concurrency: it does nothing but
// fulfil the required interface.

namespace test {

class ConcurrencyPolicy {
    /**
     * An RAII object that represents holding a critical section. Execution
     * enters the critical section on construction and leaves it on
     * destruction.
     */
    class [[nodiscard]] CriticalSection {};

  public:
    /**
     * Call a callable under a critical section.
     * Safely poll on condition (if any) before entering a critical section.
     *
     * This construct ensures that the condition can be polled on without
     * blocking higher priority tasks or interrupts from being executed.
     *
     * @param f    Callable to be executed within the critical section
     * @param pred Predicate that returns true if the callable should be called
     */
    template <std::invocable F, std::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, Pred &&...pred)
        -> decltype(std::forward<F>(f)()) {
        while (true) {
            [[maybe_unused]] CriticalSection cs{};
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
            // if pred() is false, then leave the critical section to give
            // higher priority tasks and interrupts an opportunity to be
            // serviced before checking again
        }
    }
};

} // namespace test
