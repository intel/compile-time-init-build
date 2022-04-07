cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

set(CMAKE_CXX_STANDARD 17)

add_executable(tests detail/meta.cpp builder_meta.cpp callback.cpp nexus.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tests
        PRIVATE
            -ftemplate-backtrace-limit=0
            -ferror-limit=2
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests
        PRIVATE
            -ftemplate-backtrace-limit=0
            -fmax-errors=2
    )
endif()

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain Cib)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
