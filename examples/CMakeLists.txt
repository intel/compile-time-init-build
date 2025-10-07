add_custom_target(examples)

function(transform_to_example VAR VALUE)
    cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH pp)
    cmake_path(GET pp FILENAME prefix)
    string(PREPEND VALUE "EXAMPLE.${prefix}.")
    set(${VAR}
        ${VALUE}
        PARENT_SCOPE)
endfunction()

function(add_example)
    set(singleValueArgs NAME)
    set(multiValueArgs FILES LIBS)
    cmake_parse_arguments(EX "" "${singleValueArgs}" "${multiValueArgs}"
                          ${ARGN})
    transform_to_example(EX_NAME ${EX_NAME})

    add_executable(${EX_NAME} ${EX_FILES})
    target_link_libraries(${EX_NAME} ${EX_LIBS})
    add_dependencies(examples ${EX_NAME})
endfunction()

add_subdirectory(flow)
add_subdirectory(log)
add_subdirectory(nexus)
