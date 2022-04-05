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
        ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/core.hpp)
