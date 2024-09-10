add_compile_fail_test(cyclic_flow.cpp LIBRARIES warnings cib)
add_compile_fail_test(only_reference_added.cpp LIBRARIES warnings cib)
add_compile_fail_test(mismatched_flow_runtime_conditional.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(node_explicitly_added_twice.cpp LIBRARIES warnings cib)
