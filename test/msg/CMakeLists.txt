add_tests(
    callback
    field_extract
    field_insert
    field_matchers
    handler
    handler_builder
    indexed_builder
    indexed_callback
    indexed_handler
    message
    send)

add_subdirectory(fail)

# Create a target for the current configuration
add_benchmark(handler_bench NANO FILES handler_bench.cpp SYSTEM_LIBRARIES cib)
target_compile_options(handler_bench PRIVATE -fconstexpr-steps=4000000000
                                             -fbracket-depth=1024)
