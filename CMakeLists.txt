cmake_minimum_required(VERSION 3.16)

project(
    cib
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION
        "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds are a bad idea. Please make a build directory instead."
    )
endif()

include(cmake/dependencies.cmake)
include(cmake/libraries.cmake)

add_versioned_package("gh:fmtlib/fmt#9.1.0")

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_17)
target_include_directories(cib INTERFACE include)
target_link_libraries_system(cib INTERFACE fmt::fmt-header-only)

target_compile_options(
    cib
    INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-string-literal-operator-template>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-zero-variadic-macro-arguments>)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    if(CLANG_TOOLS_PATH)
        list(APPEND CMAKE_PROGRAM_PATH ${CLANG_TOOLS_PATH})
    endif()

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    include(cmake/sanitizers.cmake)
    include(cmake/test.cmake)
    include(cmake/warnings.cmake)

    # Enable functional and performance test suites.
    add_subdirectory(test)
    add_subdirectory(benchmark)
    add_subdirectory(examples)

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

    add_versioned_package(
        NAME
        Format.cmake
        VERSION
        1.7.3
        GITHUB_REPOSITORY
        TheLartians/Format.cmake
        OPTIONS
        "CMAKE_FORMAT_EXCLUDE cmake/CPM.cmake")

    add_custom_target(quality)
    add_dependencies(quality check-clang-format check-cmake-format)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        find_program(CLANG_TIDY_PROGRAM "clang-tidy")
        if(CLANG_TIDY_PROGRAM)
            add_custom_target(clang-tidy)
            include(cmake/clang-tidy.cmake)
            clang_tidy_interface(cib)
        else()
            message(STATUS "clang-tidy not found. Adding dummy target.")
            set(CLANG_TIDY_NOT_FOUND_COMMAND_ARGS
                COMMAND ${CMAKE_COMMAND} -E echo
                "Cannot run clang-tidy because clang-tidy not found." COMMAND
                ${CMAKE_COMMAND} -E false)
            add_custom_target(clang-tidy ${CLANG_TIDY_NOT_FOUND_COMMAND_ARGS})
        endif()
        add_dependencies(quality clang-tidy)
    endif()
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
