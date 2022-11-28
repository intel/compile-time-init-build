cmake_minimum_required(VERSION 3.16)

project(cib_serial_port LANGUAGES CXX)

add_executable(cib_serial_port main.cpp)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # to fetch cib, either use CPM (https://github.com/cpm-cmake/CPM.cmake) or
    # plain old CMake functionality

    # update this to a more recent commit ID (or tag) for your project
    set(CIB_VERSION "476c583")

    if(CPM_DOWNLOAD_VERSION)
        cpmaddpackage("gh:intel/compile-time-init-build#${CIB_VERSION}")
    else()
        include(FetchContent)
        FetchContent_Declare(
            cib
            GIT_REPOSITORY https://github.com/intel/compile-time-init-build.git
            GIT_TAG ${CIB_VERSION})
        FetchContent_MakeAvailable(cib)
    endif()
endif()

target_link_libraries(cib_serial_port cib)