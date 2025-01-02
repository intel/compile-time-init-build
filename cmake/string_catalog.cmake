function(gen_str_catalog)
    set(options FORGET_OLD_IDS)
    set(oneValueArgs
        OUTPUT_CPP
        OUTPUT_XML
        OUTPUT_JSON
        GEN_STR_CATALOG
        OUTPUT_LIB
        CLIENT_NAME
        VERSION
        GUID_ID
        GUID_MASK)
    set(multiValueArgs INPUT_JSON INPUT_LIBS INPUT_HEADERS STABLE_JSON)
    cmake_parse_arguments(SC "${options}" "${oneValueArgs}" "${multiValueArgs}"
                          ${ARGN})

    list(TRANSFORM SC_INPUT_LIBS
         PREPEND "${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols_"
                 OUTPUT_VARIABLE UNDEFS)
    list(TRANSFORM UNDEFS APPEND ".txt")

    foreach(LIB UNDEF IN ZIP_LISTS SC_INPUT_LIBS UNDEFS)
        get_target_property(lib_type ${LIB} TYPE)
        if(${lib_type} STREQUAL OBJECT_LIBRARY)
            add_custom_command(
                OUTPUT ${UNDEF}
                DEPENDS ${LIB}
                COMMAND ${CMAKE_NM} -uC "$<TARGET_OBJECTS:${LIB}>" > "${UNDEF}"
                COMMAND_EXPAND_LISTS)
        else()
            add_custom_command(
                OUTPUT ${UNDEF}
                DEPENDS ${LIB}
                COMMAND ${CMAKE_NM} -uC "$<TARGET_FILE:${LIB}>" > "${UNDEF}")
        endif()
    endforeach()

    foreach(INPUT ${SC_INPUT_JSON})
        file(REAL_PATH ${INPUT} out_path)
        list(APPEND INPUT_JSON ${out_path})
    endforeach()
    foreach(INPUT ${SC_STABLE_JSON})
        file(REAL_PATH ${INPUT} out_path)
        list(APPEND STABLE_JSON ${out_path})
    endforeach()
    foreach(INPUT ${SC_INPUT_HEADERS})
        file(REAL_PATH ${INPUT} out_path)
        list(APPEND INPUT_HEADERS ${out_path})
    endforeach()

    if(SC_FORGET_OLD_IDS)
        set(FORGET_ARG "--forget_old_ids")
    endif()
    if(SC_CLIENT_NAME)
        set(CLIENT_NAME_ARG --client_name ${SC_CLIENT_NAME})
    endif()
    if(SC_VERSION)
        set(VERSION_ARG --version ${SC_VERSION})
    endif()
    if(SC_GUID_ID)
        set(GUID_ID_ARG --guid_id ${SC_GUID_ID})
    endif()
    if(SC_GUID_MASK)
        set(GUID_MASK_ARG --guid_mask ${SC_GUID_MASK})
    endif()

    add_custom_command(
        OUTPUT ${SC_OUTPUT_CPP} ${SC_OUTPUT_JSON} ${SC_OUTPUT_XML}
        COMMAND
            ${Python3_EXECUTABLE} ${SC_GEN_STR_CATALOG} --input ${UNDEFS}
            --json_input ${INPUT_JSON} --cpp_headers ${INPUT_HEADERS}
            --cpp_output ${SC_OUTPUT_CPP} --json_output ${SC_OUTPUT_JSON}
            --xml_output ${SC_OUTPUT_XML} --stable_json ${STABLE_JSON}
            ${FORGET_ARG} ${CLIENT_NAME_ARG} ${VERSION_ARG} ${GUID_ID_ARG}
            ${GUID_MASK_ARG}
        DEPENDS ${UNDEFS} ${INPUT_JSON} ${SC_GEN_STR_CATALOG} ${STABLE_JSON}
        COMMAND_EXPAND_LISTS)
    if(SC_OUTPUT_LIB)
        add_library(${SC_OUTPUT_LIB} STATIC ${SC_OUTPUT_CPP})
        target_link_libraries(${SC_OUTPUT_LIB} PUBLIC cib)
    endif()
endfunction()
