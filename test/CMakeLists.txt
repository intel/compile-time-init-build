function(add_tests)
    foreach(name ${ARGN})
        string(REPLACE "/" "_" test ${name})
        add_unit_test(
            "${test}_test"
            CATCH2
            FILES
            "${name}.cpp"
            LIBRARIES
            warnings
            cib)
    endforeach()
endfunction()

add_unit_test(
    "flow_flow_test"
    CATCH2
    FILES
    "flow/flow.cpp"
    "flow/flow_cib_func.cpp"
    LIBRARIES
    warnings
    cib)

add_tests(
    cib/builder_meta
    cib/callback
    cib/nexus
    cib/readme_hello_world
    flow/graph
    flow/graph_builder
    interrupt/dynamic_controller
    interrupt/irq_impl
    interrupt/manager
    interrupt/shared_irq_impl
    interrupt/shared_sub_irq_impl
    interrupt/sub_irq_impl
    interrupt/policies
    log/fmt_logger
    log/log
    log/mipi_encoder
    log/mipi_logger
    log/module_id
    lookup/input
    lookup/linear_search
    lookup/pseudo_pext_lookup
    lookup/lookup
    match/and
    match/constant
    match/equivalence
    match/implies
    match/not
    match/or
    match/predicate
    match/simplify_and
    match/simplify_custom
    match/simplify_not
    match/simplify_or
    match/sum_of_products
    msg/callback
    msg/field_extract
    msg/field_insert
    msg/field_matchers
    msg/handler
    msg/handler_builder
    msg/indexed_builder
    msg/indexed_callback
    msg/indexed_handler
    msg/message
    sc/format
    sc/string_constant
    seq/sequencer)

add_library(catalog1_lib STATIC log/catalog1_lib.cpp)
add_library(catalog2_lib OBJECT log/catalog2a_lib.cpp log/catalog2b_lib.cpp)
target_include_directories(catalog1_lib PRIVATE ${CMAKE_SOURCE_DIR}/test/)
target_include_directories(catalog2_lib PRIVATE ${CMAKE_SOURCE_DIR}/test/)
target_link_libraries(catalog1_lib PRIVATE warnings cib)
target_link_libraries(catalog2_lib PRIVATE warnings cib)
gen_str_catalog(
    GEN_STR_CATALOG
    ${CMAKE_SOURCE_DIR}/tools/gen_str_catalog.py
    OUTPUT_CPP
    ${CMAKE_CURRENT_BINARY_DIR}/strings.cpp
    OUTPUT_JSON
    ${CMAKE_CURRENT_BINARY_DIR}/strings.json
    OUTPUT_XML
    ${CMAKE_CURRENT_BINARY_DIR}/strings.xml
    INPUT_LIBS
    catalog1_lib
    catalog2_lib
    INPUT_JSON
    log/catalog_extra.json
    STABLE_JSON
    log/stable_strings.json
    INPUT_HEADERS
    log/catalog_enums.hpp)

add_library(catalog_strings STATIC ${CMAKE_CURRENT_BINARY_DIR}/strings.cpp)
target_link_libraries(catalog_strings PUBLIC cib)

add_unit_test(
    log_catalog_test
    CATCH2
    FILES
    log/catalog_app.cpp
    LIBRARIES
    warnings
    cib
    catalog1_lib
    catalog2_lib
    catalog_strings)

add_compile_fail_test(msg/fail/callback_bad_field_name.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/field_location.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/field_size.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/impossible_match_callback.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/impossible_match_field.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/owning_msg_incompatible_storage.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/owning_msg_incompatible_view.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/message_const_field_write.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/message_dup_fieldnames.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/message_incompatible_matcher.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/message_uninitialized_field.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/view_upsize.cpp LIBRARIES warnings cib)

add_versioned_package("gh:martinus/nanobench#9f871ad")
add_versioned_package("gh:boost-ext/mph#v1.0.8")
add_versioned_package("gh:serge-sans-paille/frozen#292a811")

# Define the variable values
set(BENCH_ALG_NAMES
    bench_std_map
    bench_std_unordered_map
    bench_frozen_map
    bench_frozen_unordered_map
    bench_mph_pext
    bench_mph
    bench_pseudo_pext_direct
    bench_pseudo_pext_indirect_1
    bench_pseudo_pext_indirect_2
    bench_pseudo_pext_indirect_3
    bench_pseudo_pext_indirect_4
    bench_pseudo_pext_indirect_5
    bench_pseudo_pext_indirect_6)

set(BENCH_DATASETS
    exp_1_uint16
    exp_2_uint16
    exp_3_uint16
    exp_4_uint16
    exp_5_uint16
    exp_6_uint16
    exp_7_uint16
    exp_8_uint16
    exp_9_uint16
    exp_10_uint16
    exp_20_uint16
    exp_30_uint16
    exp_40_uint16
    exp_50_uint16
    exp_60_uint16
    exp_70_uint16
    exp_80_uint16
    exp_90_uint16
    exp_100_uint16
    exp_200_uint16
    exp_300_uint16
    exp_400_uint16
    exp_500_uint16
    exp_600_uint16
    exp_700_uint16
    exp_800_uint16
    exp_900_uint16
    exp_1000_uint16
    exp_2000_uint16
    exp_3000_uint16
    exp_4000_uint16
    exp_5000_uint16
    exp_6000_uint16
    exp_7000_uint16
    exp_8000_uint16
    exp_9000_uint16
    exp_1_uint32
    exp_2_uint32
    exp_3_uint32
    exp_4_uint32
    exp_5_uint32
    exp_6_uint32
    exp_7_uint32
    exp_8_uint32
    exp_9_uint32
    exp_10_uint32
    exp_20_uint32
    exp_30_uint32
    exp_40_uint32
    exp_50_uint32
    exp_60_uint32
    exp_70_uint32
    exp_80_uint32
    exp_90_uint32
    exp_100_uint32
    exp_200_uint32
    exp_300_uint32
    exp_400_uint32
    exp_500_uint32
    exp_600_uint32
    exp_700_uint32
    exp_800_uint32
    exp_900_uint32
    exp_1000_uint32
    exp_2000_uint32
    exp_3000_uint32
    exp_4000_uint32
    exp_5000_uint32
    exp_6000_uint32
    exp_7000_uint32
    exp_8000_uint32
    exp_9000_uint32)

set(EXCLUDED_COMBINATIONS
    bench_mph_pext_exp_70_uint32
    bench_mph_pext_exp_80_uint32
    bench_mph_pext_exp_90_uint32
    bench_mph_pext_exp_100_uint32
    bench_mph_pext_exp_200_uint32
    bench_mph_pext_exp_300_uint32
    bench_mph_pext_exp_400_uint32
    bench_mph_pext_exp_500_uint32
    bench_mph_pext_exp_600_uint32
    bench_mph_pext_exp_700_uint32
    bench_mph_pext_exp_800_uint32
    bench_mph_pext_exp_900_uint32
    bench_mph_pext_exp_1000_uint32
    bench_mph_pext_exp_2000_uint32
    bench_mph_pext_exp_3000_uint32
    bench_mph_pext_exp_4000_uint32
    bench_mph_pext_exp_5000_uint32
    bench_mph_pext_exp_6000_uint32
    bench_mph_pext_exp_7000_uint32
    bench_mph_pext_exp_8000_uint32
    bench_mph_pext_exp_9000_uint32
    bench_mph_pext_exp_70_uint16
    bench_mph_pext_exp_80_uint16
    bench_mph_pext_exp_90_uint16
    bench_mph_pext_exp_100_uint16
    bench_mph_pext_exp_200_uint16
    bench_mph_pext_exp_300_uint16
    bench_mph_pext_exp_400_uint16
    bench_mph_pext_exp_500_uint16
    bench_mph_pext_exp_600_uint16
    bench_mph_pext_exp_700_uint16
    bench_mph_pext_exp_800_uint16
    bench_mph_pext_exp_900_uint16
    bench_mph_pext_exp_1000_uint16
    bench_mph_pext_exp_2000_uint16
    bench_mph_pext_exp_3000_uint16
    bench_mph_pext_exp_4000_uint16
    bench_mph_pext_exp_5000_uint16
    bench_mph_pext_exp_6000_uint16
    bench_mph_pext_exp_7000_uint16
    bench_mph_pext_exp_8000_uint16
    bench_mph_pext_exp_9000_uint16)

# Create a custom target to build all the benchmark binaries
add_custom_target(build_benchmarks)

# Create a custom target to run all the benchmarks
add_custom_target(run_benchmarks)

# Generate all possible combinations of the variables

foreach(BENCH_ALG_NAME ${BENCH_ALG_NAMES})
    foreach(BENCH_DATASET ${BENCH_DATASETS})
        list(FIND EXCLUDED_COMBINATIONS "${BENCH_ALG_NAME}_${BENCH_DATASET}"
             EXCLUDE_INDEX)

        if(NOT EXCLUDE_INDEX EQUAL -1)
            # Skip this combination
            continue()
        endif()

        # Generate a unique target name
        string(REPLACE "::" "_" TARGET_NAME
                       "${BENCH_ALG_NAME}_${BENCH_DATASET}")

        # Create a target for the current configuration
        add_executable(benchmark_${TARGET_NAME} lookup/pseudo_pext.cpp)
        target_link_libraries(benchmark_${TARGET_NAME} cib mph frozen-headers
                              nanobench)
        target_compile_options(
            benchmark_${TARGET_NAME} PUBLIC -O3 -march=native
                                            -fconstexpr-steps=4000000000)

        # Set the compile definitions for the current configuration
        target_compile_definitions(
            benchmark_${TARGET_NAME}
            PRIVATE ALG_NAME=${BENCH_ALG_NAME} QALG_NAME="${BENCH_ALG_NAME}"
                    BENCH_DATASET=${BENCH_DATASET}
                    QBENCH_DATASET="${BENCH_DATASET}")

        # Add the current configuration target to the build_benchmarks target
        add_dependencies(build_benchmarks benchmark_${TARGET_NAME})

        # Add a custom command to run the current benchmark
        add_custom_command(
            TARGET run_benchmarks
            POST_BUILD
            COMMAND benchmark_${TARGET_NAME}
            COMMENT "Running benchmark: ${TARGET_NAME}")
    endforeach()
endforeach()

# Create a target for the current configuration
add_executable(handler_bench msg/handler_bench.cpp)
target_link_libraries(handler_bench cib nanobench)
target_compile_options(
    handler_bench PUBLIC -O3 -march=native -fconstexpr-steps=4000000000
                         -fbracket-depth=1024)
