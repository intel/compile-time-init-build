if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_VERSION}
                                                VERSION_GREATER_EQUAL 13.2)
    add_compile_fail_test(nonexistent_flow.cpp LIBRARIES cib_interrupt)
endif()
