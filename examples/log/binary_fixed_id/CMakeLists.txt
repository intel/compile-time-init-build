cmake_minimum_required(VERSION 3.25)

project(binary_fixed_id LANGUAGES CXX)

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
    transform_to_example(binary_fixed_id_lib binary_fixed_id_lib)
    transform_to_example(binary_fixed_id_strings_lib
                         binary_fixed_id_strings_lib)
    transform_to_example(binary_fixed_id binary_fixed_id)
else()
    set(binary_fixed_id_lib binary_fixed_id_lib)
    set(binary_fixed_id_strings_lib binary_fixed_id_strings_lib)
    set(binary_fixed_id binary_fixed_id)
endif()

# build the bulk of the code as a library
add_library(${binary_fixed_id_lib} lib.cpp)
target_link_libraries(${binary_fixed_id_lib} PRIVATE cib_log_binary)

# generate the strings from that library
gen_str_catalog(
    OUTPUT_CPP
    ${CMAKE_CURRENT_BINARY_DIR}/strings.cpp
    OUTPUT_JSON
    ${CMAKE_CURRENT_BINARY_DIR}/strings.json
    OUTPUT_XML
    ${CMAKE_CURRENT_BINARY_DIR}/strings.xml
    INPUT_LIBS
    ${binary_fixed_id_lib}
    OUTPUT_LIB
    ${binary_fixed_id_strings_lib})

# link a stub executable with the "main" library and the strings library
add_executable(${binary_fixed_id} main.cpp)
target_link_libraries(${binary_fixed_id} ${binary_fixed_id_lib}
                      ${binary_fixed_id_strings_lib})
