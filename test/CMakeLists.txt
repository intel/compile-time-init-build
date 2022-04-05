cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

set(CMAKE_CXX_STANDARD 17)


add_executable(tests test_meta.cpp test_builder_meta.cpp test_callback.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain Cib)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)