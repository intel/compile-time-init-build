cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)


if (DEFINED ENV{CXX_STANDARD} AND NOT $ENV{CXX_STANDARD} EQUAL "")
    set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
message("CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

enable_testing()

add_subdirectory(lib/Catch2)
add_subdirectory(test)
add_subdirectory(benchmark)

add_custom_command(
    DEPENDS
        ${CMAKE_SOURCE_DIR}/tools/gen_release_header.py
        ${CMAKE_SOURCE_DIR}/include/cib/*
        ${CMAKE_SOURCE_DIR}/include/cib/detail/*
    COMMAND
        mkdir ${CMAKE_BINARY_DIR}/include/
    COMMAND
        mkdir ${CMAKE_BINARY_DIR}/include/cib/
    COMMAND
        python3 ${CMAKE_SOURCE_DIR}/tools/gen_release_header.py ${CMAKE_SOURCE_DIR}/include/cib/cib.hpp > ${CMAKE_BINARY_DIR}/include/cib/cib.hpp
    OUTPUT
        ${CMAKE_BINARY_DIR}/include/cib/cib.hpp
)

add_custom_target(release_header
    DEPENDS
        ${CMAKE_BINARY_DIR}/include/cib/cib.hpp)



add_library(Cib INTERFACE)

if ($ENV{SINGLE_HEADER})
    message("Using single-header version of cib.hpp.")

    add_dependencies(Cib release_header)

    target_include_directories(Cib
        INTERFACE
            ${CMAKE_BINARY_DIR}/include/)

else()
    message("Using multi-header version of cib.hpp.")

    target_include_directories(Cib
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include)

    target_sources(Cib
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include/cib/cib.hpp)
endif()