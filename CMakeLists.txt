cmake_minimum_required(VERSION 3.25)

project(
    cib
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION
        "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

include(cmake/get_cpm.cmake)
if(PROJECT_IS_TOP_LEVEL)
    cpmaddpackage("gh:intel/cicd-repo-infrastructure#main")
    update_versioned_package("gh:intel/cicd-repo-infrastructure#main")
else()
    cpmaddpackage("gh:intel/cicd-repo-infrastructure#3e2bef0")
endif()

find_package(
    Python3
    COMPONENTS Interpreter
    REQUIRED)

include(cmake/string_catalog.cmake)

add_versioned_package("gh:boostorg/mp11#boost-1.83.0")
add_versioned_package(
    NAME
    fmt
    GIT_TAG
    10.2.1
    GITHUB_REPOSITORY
    fmtlib/fmt
    OPTIONS
    "FMT_INSTALL OFF"
    "FMT_OS OFF")
add_versioned_package("gh:intel/cpp-std-extensions#44bd39b")
add_versioned_package("gh:intel/cpp-baremetal-concurrency#8d49b6d")

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_20)
target_include_directories(cib INTERFACE include)
target_link_libraries_system(cib INTERFACE concurrency fmt::fmt-header-only
                             stdx)

target_compile_options(
    cib
    INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-string-literal-operator-template>)

if(PROJECT_IS_TOP_LEVEL)
    add_docs(docs)
    clang_tidy_interface(cib)

    # Enable functional and performance test suites.
    add_subdirectory(test)
    add_subdirectory(benchmark)
    add_subdirectory(examples)
    add_subdirectory(tools)

    # Build single-header release.
    file(GLOB_RECURSE include_files
         "${CMAKE_CURRENT_SOURCE_DIR}/include/cib/*.hpp")
    add_custom_command(
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py
                ${include_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_CURRENT_BINARY_DIR}/include/cib
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_release_header.py
            ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/cib.hpp >
            ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp)

    add_custom_target(release_header
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/cib/cib.hpp)
endif()

if(DEFINED ENV{SINGLE_HEADER})
    add_dependencies(cib release_header)

    target_include_directories(
        cib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
else()
    target_include_directories(
        cib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
endif()
