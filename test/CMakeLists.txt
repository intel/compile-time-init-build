function(add_tests)
    set(multiValueArgs FILES LIBRARIES)
    cmake_parse_arguments(AT "" "" "${multiValueArgs}" ${ARGN})

    get_filename_component(prefix ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    foreach(name ${AT_FILES})
        string(REPLACE "/" "_" test ${name})
        add_unit_test(
            "${prefix}_${test}_test"
            CATCH2
            FILES
            "${name}.cpp"
            LIBRARIES
            warnings
            ${AT_LIBRARIES})
    endforeach()
endfunction()

add_subdirectory(cib)
add_subdirectory(flow)
add_subdirectory(interrupt)
add_subdirectory(log)
add_subdirectory(log_binary)
add_subdirectory(log_fmt)
add_subdirectory(lookup)
add_subdirectory(match)
add_subdirectory(msg)
add_subdirectory(nexus)
add_subdirectory(seq)
