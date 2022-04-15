cmake_minimum_required(VERSION 3.16)

project(cib_example_bare_cmake)

add_executable(cib_example_bare_cmake main.cpp)

include(FetchContent)
FetchContent_Declare(
    cib
    GIT_REPOSITORY https://github.com/intel/compile-time-init-build.git
    GIT_TAG        28e97c097da70598a5a166be68edd15925c86226
)
FetchContent_MakeAvailable(cib)

find_package(cib REQUIRED)
target_link_libraries(cib_example_bare_cmake cib::cib)