#ifndef CIB_INTERRUPT_MANAGER_INTERFACE_HPP
#define CIB_INTERRUPT_MANAGER_INTERFACE_HPP

namespace interrupt {
/**
 * Type-erased interface to the interrupt manager.
 */
class manager_interface {
  public:
    virtual void init() const = 0;
    virtual void init_mcu_interrupts() const = 0;
    virtual void init_sub_interrupts() const = 0;
};
} // namespace interrupt

#endif // CIB_INTERRUPT_MANAGER_INTERFACE_HPP
