interrupt::manager
------------------

interrupt::manager is intended to automate and hide the low level details of interrupt
hardware initialization, registration, and execution while using the least amount of memory
and execution time.

The interrupt configuration is declared using a template specialization of
interrupt::manager. The template parameters define the interrupt numbers, priorities,
and shared irq parameters like enable and status registers.

Each irq contains a flow::impl for registering interrupt service routines.

The interrupt::manager_impl::init() method enables and initializes all interrupts with associated
interrupt service routines registered.

The interrupt::manager_impl::run<irq_number>() method clears the pending interrupt status and runs any
registered interrupt service routines for that IRQ. If it's a shared interrupt, each
registered sub_irq will have its interrupt status field checked to determine if its
interrupt service routines should be executed.



