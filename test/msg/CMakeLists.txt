add_tests(
    FILES
    callback
    field_extract
    field_insert
    field_matchers
    handler
    handler_builder
    handler_uninit
    indexed_builder
    indexed_callback
    indexed_handler
    indexed_handler_uninit
    message
    relaxed_message
    LIBRARIES
    warnings
    cib_log_fmt
    cib_msg
    cib_nexus)

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
   OR ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 19)
    add_tests(
        FILES
        send
        LIBRARIES
        warnings
        cib_log_fmt
        cib_msg
        cib_nexus)
endif()

add_subdirectory(fail)
