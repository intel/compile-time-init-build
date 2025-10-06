add_custom_target(examples)

function(add_example)
    set(singleValueArgs NAME)
    set(multiValueArgs FILES)
    cmake_parse_arguments(EX "" "${singleValueArgs}" "${multiValueArgs}"
                          ${ARGN})
    string(PREPEND EX_NAME "EXAMPLE.")

    add_executable(${EX_NAME} ${EX_FILES})
    target_link_libraries(${EX_NAME} cib)
    add_dependencies(examples ${EX_NAME})
endfunction()

add_subdirectory(flow)
add_subdirectory(log)
add_subdirectory(nexus)
