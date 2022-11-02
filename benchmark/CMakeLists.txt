add_executable(compilation_benchmark big_nexus.cpp)

target_compile_options(
    compilation_benchmark
    PRIVATE -ftemplate-backtrace-limit=0
            -ftime-report
            $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=8>
            $<$<CXX_COMPILER_ID:Clang>:-ftime-trace>
            $<$<CXX_COMPILER_ID:Clang>:-ftime-trace-granularity=10>
            $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=8>)

target_link_libraries(compilation_benchmark PRIVATE cib)
target_include_directories(compilation_benchmark
                           PRIVATE ${CMAKE_SOURCE_DIR}/test/)
