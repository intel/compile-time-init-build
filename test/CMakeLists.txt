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
    log/catalog_enums.hpp
    CLIENT_NAME
    "test"
    VERSION
    "test version"
    GUID_ID
    "01234567-89ab-cdef-0123-456789abcdef"
    GUID_MASK
    "ffffffff-ffff-ffff-ffff-ffffffffffff")

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

add_compile_fail_test(flow/fail/cyclic_flow.cpp LIBRARIES warnings cib)
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
add_compile_fail_test(msg/fail/message_cmp_owner.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/message_cmp_view.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/message_const_field_write.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/message_dangling_view.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/message_dup_fieldnames.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/message_incompatible_matcher.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/message_uninitialized_field.cpp LIBRARIES
                      warnings cib)
add_compile_fail_test(msg/fail/view_upsize.cpp LIBRARIES warnings cib)

add_versioned_package("gh:boost-ext/mph#v1.0.8")
add_versioned_package("gh:serge-sans-paille/frozen#292a811")

# Define the variable values
set(BENCH_ALG_NAMES
    std_map
    std_unordered_map
    frozen_map
    frozen_unordered_map
    pseudo_pext_direct
    pseudo_pext_indirect_1
    pseudo_pext_indirect_2
    pseudo_pext_indirect_3
    pseudo_pext_indirect_4
    pseudo_pext_indirect_5
    pseudo_pext_indirect_6)

foreach(type IN ITEMS uint16 uint32)
    foreach(i RANGE 1 10)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
    foreach(i RANGE 20 100 10)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
    foreach(i RANGE 200 1000 100)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
endforeach()

set(EXCLUDED_COMBINATIONS
    mph_pext_exp_70_uint32
    mph_pext_exp_80_uint32
    mph_pext_exp_90_uint32
    mph_pext_exp_100_uint32
    mph_pext_exp_200_uint32
    mph_pext_exp_300_uint32
    mph_pext_exp_400_uint32
    mph_pext_exp_500_uint32
    mph_pext_exp_600_uint32
    mph_pext_exp_700_uint32
    mph_pext_exp_800_uint32
    mph_pext_exp_900_uint32
    mph_pext_exp_1000_uint32
    mph_pext_exp_70_uint16
    mph_pext_exp_80_uint16
    mph_pext_exp_90_uint16
    mph_pext_exp_100_uint16
    mph_pext_exp_200_uint16
    mph_pext_exp_300_uint16
    mph_pext_exp_400_uint16
    mph_pext_exp_500_uint16
    mph_pext_exp_600_uint16
    mph_pext_exp_700_uint16
    mph_pext_exp_800_uint16
    mph_pext_exp_900_uint16
    mph_pext_exp_1000_uint16)

# Generate all possible combinations of the variables
foreach(BENCH_ALG_NAME ${BENCH_ALG_NAMES})
    foreach(BENCH_DATASET ${BENCH_DATASETS})
        if("${BENCH_ALG_NAME}_${BENCH_DATASET}" IN_LIST EXCLUDED_COMBINATIONS)
            continue()
        endif()
        set(name "${BENCH_ALG_NAME}_${BENCH_DATASET}_bench")

        add_benchmark(
            ${name}
            NANO
            FILES
            lookup/pseudo_pext.cpp
            SYSTEM_LIBRARIES
            cib
            mph
            frozen-headers)
        target_compile_features(${name} PRIVATE cxx_std_23)
        target_compile_options(${name} PRIVATE -fconstexpr-steps=4000000000)
        target_compile_definitions(
            ${name}
            PRIVATE ALG_NAME=bench_${BENCH_ALG_NAME}
                    QALG_NAME="bench_${BENCH_ALG_NAME}"
                    BENCH_DATASET=${BENCH_DATASET}
                    QBENCH_DATASET="${BENCH_DATASET}")
    endforeach()
endforeach()

# Create a target for the current configuration
add_benchmark(handler_bench NANO FILES msg/handler_bench.cpp SYSTEM_LIBRARIES
              cib)
target_compile_options(handler_bench PRIVATE -fconstexpr-steps=4000000000
                                             -fbracket-depth=1024)
