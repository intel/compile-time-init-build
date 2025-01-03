add_benchmark(handler_bench NANO FILES handler_bench.cpp SYSTEM_LIBRARIES cib)
target_compile_options(
    handler_bench
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fconstexpr-steps=4000000000>
        $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=4000000000>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fbracket-depth=1024>
)
