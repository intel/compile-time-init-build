#pragma once

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
     *
     * @param callable Callable to be executed within the critical section
     */
    template <typename CallableT>
    static inline auto call_in_critical_section(CallableT &&callable)
        -> decltype(auto) {
        [[maybe_unused]] CriticalSection cs{};
        return std::forward<CallableT>(callable)();
    }

    /**
     * Safely poll on condition before entering a critical section.
     *
     * This construct ensures that the condition can be polled on without
     * blocking higher priority tasks or interrupts from being executed.
     *
     * @param predicate Callable that returns true if the critical section can
     *                  be entered
     * @param callable  Callable to be executed within the critical section
     */
    template <typename PredicateT, typename CallableT>
    static inline auto call_in_critical_section(PredicateT &&predicate,
                                                CallableT &&callable)
        -> decltype(auto) {
        while (true) {
            [[maybe_unused]] CriticalSection cs{};
            if (predicate()) {
                return std::forward<CallableT>(callable)();
            }
            // if predicate() is false, then re-enable interrupts to give
            // higher priority tasks and interrupts an opportunity to be
            // serviced before checking again
        }
    }
};

} // namespace test
