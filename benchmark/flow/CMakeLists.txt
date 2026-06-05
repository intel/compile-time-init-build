add_executable(compilation_flow_benchmark EXCLUDE_FROM_ALL compilation.cpp)

target_compile_options(
    compilation_flow_benchmark
    PRIVATE -ftemplate-backtrace-limit=0
            $<$<CXX_COMPILER_ID:Clang>:-fconstexpr-steps=2000000>
            $<$<CXX_COMPILER_ID:Clang>:-fbracket-depth=512>
            $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=8>
            $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=8>)

target_link_libraries(compilation_flow_benchmark PRIVATE cib_flow cib_nexus
                                                         profile-compilation)

add_benchmark(
    run_flow_benchmark
    NANO
    FILES
    run_bench.cpp
    SYSTEM_LIBRARIES
    cib_flow
    cib_nexus)
target_compile_options(
    run_flow_benchmark
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fconstexpr-steps=4000000000>
        $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=4000000000>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fbracket-depth=1024>
)

add_benchmark(flow_inline_benchmark FILES inline_bench.cpp SYSTEM_LIBRARIES
              cib_flow cib_nexus)
target_compile_options(
    flow_inline_benchmark
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fconstexpr-steps=4000000000>
        $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=4000000000>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fbracket-depth=1024>
)
