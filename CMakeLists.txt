cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PROJECT_NAME)
    set(NOT_SUBPROJECT ON)
else()
    set(NOT_SUBPROJECT OFF)
endif()

project(
    cib
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_17)



if(NOT_SUBPROJECT)
    if (DEFINED ENV{CXX_STANDARD} AND NOT $ENV{CXX_STANDARD} EQUAL "")
        set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
    else()
        set(CMAKE_CXX_STANDARD 17)
    endif()

    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        message("Updating git submodules...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_RETVAL)
        if(NOT GIT_RETVAL EQUAL "0")
            message(FATAL_ERROR "git submodule update failed!")
        endif()
    endif()

    # Enable functional and performance test suites.
    enable_testing()
    add_subdirectory(lib/Catch2)
    add_subdirectory(test)
    add_subdirectory(benchmark)

    # Build single-header release.
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cib/)

    find_package(PythonInterp 3 REQUIRED)

    add_custom_command(
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py
            ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/*
            ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/detail/*
        COMMAND
            python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/cib.hpp > ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp
    )

    add_custom_target(release_header
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp)
endif()

if ($ENV{SINGLE_HEADER})
    add_dependencies(cib release_header)

    target_include_directories(cib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )
else()
    target_include_directories(cib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )
endif()
