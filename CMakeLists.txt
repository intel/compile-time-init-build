cmake_minimum_required(VERSION 3.16)

project(
    cib
    VERSION 0.1.0
    DESCRIPTION "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_17)

if (DEFINED ENV{CXX_STANDARD} AND NOT $ENV{CXX_STANDARD} EQUAL "")
    set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cib/)

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


include(GNUInstallDirs)

install(
    TARGETS cib
    EXPORT cib_targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "cib_config_version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/cib_config.cmake.in"
    "${PROJECT_BINARY_DIR}/cib_config.cmake"
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/cib/cmake
)

install(
    EXPORT cib_targets
    FILE cib_targets.cmake
    NAMESPACE cib::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cib/cmake
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/cib_config.cmake"
        "${PROJECT_BINARY_DIR}/cib_config_version.cmake"
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/cib/cmake
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/cib
    DESTINATION include
)