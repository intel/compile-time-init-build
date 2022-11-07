function(clang_tidy_header HEADER TARGET)
    file(RELATIVE_PATH CT_NAME ${CMAKE_SOURCE_DIR} ${HEADER})
    string(REPLACE "/" "_" CT_NAME ${CT_NAME})
    get_filename_component(CT_NAME ${CT_NAME} NAME_WLE)
    set(CT_NAME "clang_tidy_${CT_NAME}")
    set(CPP_NAME
        "${CMAKE_BINARY_DIR}/generated-sources/${TARGET}/${CT_NAME}.cpp")

    add_custom_command(
        OUTPUT "${CPP_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/generated-sources/${TARGET}"
        COMMAND ${CMAKE_SOURCE_DIR}/cmake/create-clang-tidiable.sh ${CPP_NAME}
                ${HEADER}
        DEPENDS ${HEADER} ${CMAKE_SOURCE_DIR}/.clang-tidy)

    add_library(${CT_NAME} ${CPP_NAME})
    target_link_libraries(${CT_NAME} PRIVATE ${TARGET})
    set_target_properties(
        ${CT_NAME}
        PROPERTIES
            CXX_CLANG_TIDY
            "${CLANG_TIDY_PROGRAM};-p;${CMAKE_BINARY_DIR};-header-filter=${HEADER}"
    )

    add_dependencies(clang-tidy "${CT_NAME}")
endfunction()

function(clang_tidy_interface TARGET)
    get_target_property(DIRS ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(SYSTEM_DIRS ${TARGET}
                        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
    foreach(DIR ${DIRS})
        if(NOT DIR IN_LIST SYSTEM_DIRS)
            file(GLOB_RECURSE HEADERS "${DIR}/*.hpp")
            foreach(HEADER ${HEADERS})
                clang_tidy_header(${HEADER} ${TARGET})
            endforeach()
        endif()
    endforeach()
endfunction()
