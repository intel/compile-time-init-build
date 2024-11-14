if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION}
                                               VERSION_LESS 13)
    message(
        WARNING
            "GCC has a constexpr bug before version 13 that prevents flow steps from compiling. Not adding flow tests."
    )
else()
    add_unit_test(
        "flow_separate_actions_test"
        CATCH2
        FILES
        "flow_cib_func.cpp"
        "separate_actions.cpp"
        LIBRARIES
        warnings
        cib)

    add_tests(flow logging log_levels custom_log_levels)
endif()

add_tests(graph graph_builder)

add_subdirectory(fail)
