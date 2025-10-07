cmake_minimum_required(VERSION 3.25)

project(serial_port LANGUAGES CXX)

if(NOT TARGET cib)
    # to fetch cib, either use CPM (https://github.com/cpm-cmake/CPM.cmake) or
    # use plain old CMake functionality
    set(USE_CPM 1)

    set(CIB_VERSION "c388a4d") # update this to a more recent commit ID (or tag)
                               # for your project

    if(USE_CPM)
        file(
            DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
            EXPECTED_HASH
                SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
        )
        include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

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

if(COMMAND add_example)
    add_example(NAME serial_port FILES main.cpp LIBS cib)
else()
    add_executable(serial_port main.cpp)
    target_link_libraries(serial_port cib)
endif()
