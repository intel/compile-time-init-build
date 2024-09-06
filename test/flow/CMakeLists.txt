add_unit_test(
    "flow_flow_test"
    CATCH2
    FILES
    "flow.cpp"
    "flow_cib_func.cpp"
    LIBRARIES
    warnings
    cib)

add_tests(graph graph_builder)

add_subdirectory(fail)
