function(gen_single_header)
    set(oneValueArgs TARGET GEN_HEADER SOURCE_TARGET SOURCE_FILESET
                     INPUT_HEADER OUTPUT_HEADER)
    cmake_parse_arguments(SH "" "${oneValueArgs}" "" ${ARGN})

    get_target_property(HEADERS ${SH_SOURCE_TARGET}
                        HEADER_SET_${SH_SOURCE_FILESET})
    get_filename_component(OUTPUT_DIR ${SH_OUTPUT_HEADER} DIRECTORY)
    add_custom_command(
        DEPENDS ${SH_GEN_HEADER} ${HEADERS}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMAND ${Python3_EXECUTABLE} ${SH_GEN_HEADER} ${SH_INPUT_HEADER} >
                ${SH_OUTPUT_HEADER}
        OUTPUT ${SH_OUTPUT_HEADER})

    add_custom_target(${SH_TARGET} DEPENDS ${SH_OUTPUT_HEADER})
endfunction()
