cmake_minimum_required(VERSION 3.16)

project(hello_world)

add_executable(hello_world main.cpp)

include(FetchContent)
FetchContent_Declare(
    cib
    GIT_REPOSITORY https://github.com/intel/compile-time-init-build.git
    GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(cib)

target_link_libraries(hello_world cib)