#pragma once

namespace interrupt {
/**
 * Type-erased interface to the interrupt manager.
 */
// NOLINTNEXTLINE(cppcoreguidelines-virtual-class-destructor)
class manager_interface {
  public:
    virtual void init() const = 0;
    virtual void init_mcu_interrupts() const = 0;
    virtual void init_sub_interrupts() const = 0;
};
} // namespace interrupt
