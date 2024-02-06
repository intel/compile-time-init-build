function(gen_str_catalog)
    set(options "")
    set(oneValueArgs OUTPUT_CPP OUTPUT_XML OUTPUT_JSON GEN_STR_CATALOG
                     OUTPUT_LIB)
    set(multiValueArgs INPUT_JSON INPUT_LIBS)
    cmake_parse_arguments(SC "${options}" "${oneValueArgs}" "${multiValueArgs}"
                          ${ARGN})

    list(TRANSFORM SC_INPUT_LIBS
         PREPEND "${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols_"
                 OUTPUT_VARIABLE UNDEFS)
    list(TRANSFORM UNDEFS APPEND ".txt")

    foreach(LIB UNDEF IN ZIP_LISTS SC_INPUT_LIBS UNDEFS)
        add_custom_command(
            OUTPUT ${UNDEF}
            DEPENDS ${LIB}
            COMMAND ${CMAKE_NM} -uC "$<TARGET_FILE:${LIB}>" > "${UNDEF}")
    endforeach()

    list(TRANSFORM SC_INPUT_JSON PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

    add_custom_command(
        OUTPUT ${SC_OUTPUT_CPP} ${SC_OUTPUT_JSON} ${SC_OUTPUT_XML}
        COMMAND
            ${Python3_EXECUTABLE} ${SC_GEN_STR_CATALOG} --input ${UNDEFS}
            --json_input ${SC_INPUT_JSON} --cpp_output ${SC_OUTPUT_CPP}
            --json_output ${SC_OUTPUT_JSON} --xml_output ${SC_OUTPUT_XML}
        DEPENDS ${UNDEFS} ${INPUT_JSON} ${SC_GEN_STR_CATALOG})

    add_library(${SC_OUTPUT_LIB} STATIC ${SC_OUTPUT_CPP})
    target_link_libraries(${SC_OUTPUT_LIB} PUBLIC cib)
endfunction()
