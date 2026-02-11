add_versioned_package("gh:intel/generic-register-operation-optimizer#bfc5b64")

add_tests(
    FILES
    dynamic_controller
    id_irq_impl
    irq_impl
    manager
    shared_irq_impl
    shared_sub_irq_impl
    sub_irq_impl
    policies
    LIBRARIES
    cib_interrupt
    groov)
