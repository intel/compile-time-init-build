cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

add_library(external_lib INTERFACE IMPORTED GLOBAL)

add_subdirectory(Catch2)
add_subdirectory(fmt)

target_link_libraries(external_lib INTERFACE Catch2::Catch2WithMain cib)
target_link_libraries(external_lib INTERFACE fmt::fmt)

# ensure catch2 headers do not produce warnings
target_include_directories(external_lib SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Catch2/src/)
target_include_directories(external_lib SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/include/)

# To provide function `catch_discover_tests` to get tests from a target 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Catch2/extras)
include(Catch)