if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_VERSION}
                                                VERSION_GREATER_EQUAL 13.2)
    add_compile_fail_test(cyclic_flow.cpp LIBRARIES cib_flow cib_nexus)
    add_compile_fail_test(debug_subgraph.cpp LIBRARIES cib_flow cib_nexus)
    add_compile_fail_test(only_reference_added.cpp LIBRARIES cib_flow cib_nexus)
    add_compile_fail_test(mismatched_flow_runtime_conditional.cpp LIBRARIES
                          cib_flow cib_nexus)
    add_compile_fail_test(node_explicitly_added_twice.cpp LIBRARIES cib_flow
                          cib_nexus)
endif()
