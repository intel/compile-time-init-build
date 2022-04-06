cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_subdirectory(lib/Catch2)
add_subdirectory(test)

add_library(Cib INTERFACE)

target_include_directories(Cib
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(Cib
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/cib.hpp)


add_custom_command(
    DEPENDS
        ${CMAKE_SOURCE_DIR}/include/cib/*
        ${CMAKE_SOURCE_DIR}/include/cib/detail/*
    COMMAND
        python3 ${CMAKE_SOURCE_DIR}/tools/gen_release_header.py ${CMAKE_SOURCE_DIR}/include/cib/cib.hpp > ${CMAKE_BINARY_DIR}/cib.hpp
    OUTPUT
        ${CMAKE_BINARY_DIR}/cib.hpp
)

add_custom_target(release
    DEPENDS
        ${CMAKE_BINARY_DIR}/cib.hpp)