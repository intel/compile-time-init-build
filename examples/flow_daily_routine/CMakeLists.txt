cmake_minimum_required(VERSION 3.16)

project(flow_daily_routine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

add_executable(flow_daily_routine main.cpp)

target_link_libraries(flow_daily_routine cib)