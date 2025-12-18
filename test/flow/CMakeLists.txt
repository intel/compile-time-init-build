add_unit_test(
    "flow_separate_actions_test"
    CATCH2
    FILES
    "flow_cib_func.cpp"
    "separate_actions.cpp"
    LIBRARIES
    warnings
    cib_flow
    cib_nexus)

add_tests(
    FILES
    builder
    flow
    flow_uninit
    graph_builder
    logging
    log_levels
    custom_log_levels
    LIBRARIES
    cib_flow
    cib_log_fmt
    cib_nexus)

add_subdirectory(fail)

function(add_debug_flow_test)
    add_executable(debug_flow EXCLUDE_FROM_ALL debug_flow.cpp)
    target_link_libraries(debug_flow PRIVATE cib_flow cib_nexus)
    generate_flow_graph(
        TARGET
        debug_flow
        FLOW_NAME
        test
        START_NODE
        b
        END_NODE
        c)

    if(TARGET debug_flow.test.graph)
        add_custom_target(run_debug_flow_test DEPENDS debug_flow_test.passed)
        add_custom_command(
            OUTPUT debug_flow_test.passed
            COMMAND diff ${CMAKE_CURRENT_BINARY_DIR}/debug_flow.test.graph.mmd
                    ${CMAKE_CURRENT_SOURCE_DIR}/debug_flow.test.graph.mmd
            COMMAND ${CMAKE_COMMAND} "-E" "touch" "debug_flow_test.passed"
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/debug_flow.test.graph.mmd
                    ${CMAKE_CURRENT_SOURCE_DIR}/debug_flow.test.graph.mmd)
        add_dependencies(${INFRA_TARGET_NAMESPACE}build_unit_tests
                         run_debug_flow_test)
        add_dependencies(${INFRA_TARGET_NAMESPACE}cpp_tests run_debug_flow_test)

        add_test(
            NAME debug_flow_test
            COMMAND diff ${CMAKE_CURRENT_BINARY_DIR}/debug_flow.test.graph.mmd
                    ${CMAKE_CURRENT_SOURCE_DIR}/debug_flow.test.graph.mmd)
    endif()
endfunction()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_CXX_COMPILER_VERSION}
                                                 VERSION_GREATER_EQUAL 15)
    add_debug_flow_test()
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION}
                                               VERSION_GREATER_EQUAL 13.2)
    add_debug_flow_test()
endif()
