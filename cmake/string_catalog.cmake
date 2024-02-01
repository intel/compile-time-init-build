function(gen_str_catalog)
    set(options "")
    set(oneValueArgs OUTPUT_CPP OUTPUT_XML OUTPUT_JSON GEN_STR_CATALOG
                     OUTPUT_LIB)
    set(multiValueArgs INPUT_LIBS)
    cmake_parse_arguments(SC "${options}" "${oneValueArgs}" "${multiValueArgs}"
                          ${ARGN})

    foreach(X IN LISTS SC_INPUT_LIBS)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt
            DEPENDS ${X}
            COMMAND ${CMAKE_NM} -uC "lib${X}.a" >
                    ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt)
    endforeach()

    add_custom_command(
        OUTPUT ${SC_OUTPUT_CPP} ${SC_OUTPUT_JSON} ${SC_OUTPUT_XML}
        COMMAND
            ${Python3_EXECUTABLE} ${SC_GEN_STR_CATALOG} --input
            ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt --cpp_output
            ${SC_OUTPUT_CPP} --json_output ${SC_OUTPUT_JSON} --xml_output
            ${SC_OUTPUT_XML}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt
                ${SC_GEN_STR_CATALOG})

    add_library(${SC_OUTPUT_LIB} STATIC ${SC_OUTPUT_CPP})
    target_link_libraries(${SC_OUTPUT_LIB} PUBLIC cib)
endfunction()
