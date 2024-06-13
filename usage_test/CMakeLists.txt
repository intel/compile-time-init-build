if(DEFINED ENV{USER_CMAKE_VERSION})
    message(STATUS "Required minimum cmake version: $ENV{USER_CMAKE_VERSION}")
    cmake_minimum_required(VERSION $ENV{USER_CMAKE_VERSION})
endif()
message(STATUS "Actual cmake version: ${CMAKE_VERSION}")

project(cib_usage)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/get_cpm.cmake)
cpmaddpackage(NAME compile-time-init-build SOURCE_DIR
              "${CMAKE_CURRENT_LIST_DIR}/.." GIT_TAG HEAD)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE cib)
