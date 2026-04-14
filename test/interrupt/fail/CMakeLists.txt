if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_CXX_COMPILER_VERSION}
                                                 VERSION_GREATER_EQUAL 15)
    add_compile_fail_test(nonexistent_flow.cpp LIBRARIES cib_interrupt)
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION}
                                               VERSION_GREATER_EQUAL 13.2)
    add_compile_fail_test(nonexistent_flow.cpp LIBRARIES cib_interrupt)
endif()
