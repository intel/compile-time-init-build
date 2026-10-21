if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_VERSION}
                                                VERSION_GREATER_EQUAL 13.2)
    add_compile_fail_test(extend_unexported_service.cpp LIBRARIES cib_nexus)
endif()

add_compile_fail_test(moveonly_multiple_callbacks.cpp LIBRARIES cib_nexus)
