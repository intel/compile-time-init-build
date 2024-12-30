add_benchmark(handler_bench NANO FILES handler_bench.cpp SYSTEM_LIBRARIES cib)
target_compile_options(handler_bench PRIVATE -fconstexpr-steps=4000000000
                                             -fbracket-depth=1024)
