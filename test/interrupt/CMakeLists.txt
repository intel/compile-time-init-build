add_versioned_package("gh:intel/generic-register-operation-optimizer#d365927")

add_tests(
    FILES
    dynamic_controller
    irq_impl
    manager
    shared_irq_impl
    shared_sub_irq_impl
    sub_irq_impl
    policies
    LIBRARIES
    warnings
    cib_interrupt
    groov)

add_subdirectory(fail)
