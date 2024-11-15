add_unit_test(
    "flow_separate_actions_test"
    CATCH2
    FILES
    "flow_cib_func.cpp"
    "separate_actions.cpp"
    LIBRARIES
    warnings
    cib)

add_tests(flow graph graph_builder logging log_levels custom_log_levels)

add_subdirectory(fail)
