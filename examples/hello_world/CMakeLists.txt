cmake_minimum_required(VERSION 3.16)

project(hello_world LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(hello_world main.cpp dont_panic.cpp)

include(FetchContent)
FetchContent_Declare(
    cib
    GIT_REPOSITORY https://github.com/intel/compile-time-init-build.git
    # update this to a more recent commit ID for your project
    GIT_TAG 8b35ed8f6fb358234a916768c81a924a373c80ff)
FetchContent_MakeAvailable(cib)

target_link_libraries(hello_world cib)
