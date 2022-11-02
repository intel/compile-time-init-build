#pragma once

// NOTE: this is a mock implementation of concurrency

namespace conc {
template <typename Callable> inline void critical_section(Callable callable) {
    callable();
};

/**
 * Safely poll on condition before entering a critical section.
 *
 * This construct ensures that the condition can be polled on without blocking
 * higher priority tasks or interrupts from being executed.
 *
 * @param condition Callable that returns true if the critical section can be
 * entered
 * @param callable  Callable to be executed within the critical section
 */
template <typename ConditionType, typename CallableType>
inline void critical_section(ConditionType condition, CallableType callable) {
    uint32_t ieValue;

    while (true) {
        if (condition()) {
            // execute the main body of the critical section
            callable();
            return;
        } else {
            // if condition() is false, then re-enable interrupts to give higher
            // priority tasks and interrupts an opportunity to be serviced
            // before checking condition() again
        }
    }
};

class CriticalSection {
  private:
    uint32_t ieValue;

  public:
    CriticalSection() {}

    ~CriticalSection() {}
};

} // namespace conc
