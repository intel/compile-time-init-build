cmake_minimum_required(VERSION 3.16)

add_executable(compilation_benchmark
    big_nexus.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(compilation_benchmark
        PRIVATE
            -ferror-limit=8
            -ftime-report
            -ftime-trace
            -ftime-trace-granularity=10
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(compilation_benchmark
        PRIVATE
            -fmax-errors=8
            -ftime-report
    )
endif()


target_compile_options(compilation_benchmark
    PRIVATE
        -ftemplate-backtrace-limit=0
)

target_link_libraries_system(compilation_benchmark PRIVATE fmt::fmt)
target_link_libraries(compilation_benchmark PRIVATE cib)
target_include_directories(compilation_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/test/)
