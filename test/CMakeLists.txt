function(add_tests)
    get_filename_component(prefix ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    foreach(name ${ARGN})
        string(REPLACE "/" "_" test ${name})
        add_unit_test(
            "${prefix}_${test}_test"
            CATCH2
            FILES
            "${name}.cpp"
            LIBRARIES
            warnings
            cib)
    endforeach()
endfunction()

add_subdirectory(cib)
add_subdirectory(flow)
add_subdirectory(interrupt)
add_subdirectory(log)
add_subdirectory(lookup)
add_subdirectory(match)
add_subdirectory(msg)
add_subdirectory(sc)
add_subdirectory(seq)
