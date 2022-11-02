cmake_minimum_required(VERSION 3.16)

project(
    cib
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION
        "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

include(cmake/dependencies.cmake)
include(cmake/libraries.cmake)

add_versioned_package("gh:boostorg/hana#b804937")
add_versioned_package("gh:fmtlib/fmt#9.1.0")

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_17)
target_include_directories(cib INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cib SYSTEM INTERFACE ${hana_SOURCE_DIR}/include)
target_link_libraries_system(cib INTERFACE fmt::fmt)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Enable functional and performance test suites.
    enable_testing()
    add_subdirectory(test)
    add_subdirectory(benchmark)

    # Build single-header release.
    find_package(PythonInterp 3 REQUIRED)

    file(GLOB_RECURSE include_files
         "${CMAKE_CURRENT_SOURCE_DIR}/include/cib/*.hpp")
    add_custom_command(
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py
                ${include_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_CURRENT_BINARY_DIR}/include/cib
        COMMAND
            python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py
            ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/cib.hpp >
            ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp)

    add_custom_target(release_header
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp)
endif()

if($ENV{SINGLE_HEADER})
    add_dependencies(cib release_header)

    target_include_directories(
        cib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
else()
    target_include_directories(
        cib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
endif()

function(gen_str_catalog)
    set(options "")

    set(oneValueArgs OUTPUT_CPP OUTPUT_XML OUTPUT_JSON GEN_STR_CATALOG)

    set(multiValueArgs INPUT_LIBS)

    cmake_parse_arguments(SC "${options}" "${oneValueArgs}" "${multiValueArgs}"
                          ${ARGN})

    foreach(X IN LISTS SC_INPUT_LIBS)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt
            DEPENDS ${X}
            COMMAND nm -uC ${X} >>
                    ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt)
    endforeach()

    add_custom_command(
        OUTPUT ${SC_OUTPUT_CPP} ${SC_OUTPUT_JSON} ${SC_OUTPUT_XML}
        COMMAND
            python3 ${SC_GEN_STR_CATALOG}
            ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt ${SC_OUTPUT_CPP}
            ${SC_OUTPUT_JSON} ${SC_OUTPUT_XML}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/undefined_symbols.txt
                ${SC_GEN_STR_CATALOG})

    add_library(log_strings STATIC strings.cpp)
    target_link_libraries(log_strings PUBLIC cib)
endfunction()
