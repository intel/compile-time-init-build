add_executable(compilation_benchmark EXCLUDE_FROM_ALL big_nexus.cpp)

target_compile_options(
    compilation_benchmark
    PRIVATE -ftemplate-backtrace-limit=0
            $<$<CXX_COMPILER_ID:Clang>:-fconstexpr-steps=2000000>
            $<$<CXX_COMPILER_ID:Clang>:-fbracket-depth=512>
            $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=8>
            $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=8>)

target_link_libraries(compilation_benchmark PRIVATE cib profile-compilation)
target_include_directories(compilation_benchmark
                           PRIVATE ${CMAKE_SOURCE_DIR}/test/)
