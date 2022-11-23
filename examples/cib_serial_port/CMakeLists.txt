cmake_minimum_required(VERSION 3.16)

project(cib_serial_port LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

add_executable(cib_serial_port main.cpp)

target_link_libraries(cib_serial_port cib)