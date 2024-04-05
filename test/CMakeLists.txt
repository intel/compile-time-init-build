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
    lookup/direct_array
    lookup/fast_hash
    lookup/input
    lookup/linear_search
    lookup/lookup
    lookup/strategies
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
    msg/detail/rle_codec
    msg/field_extract
    msg/field_insert
    msg/field_matchers
    msg/handler
    msg/handler_builder
    msg/indexed_builder
    msg/indexed_callback
    msg/indexed_handler
    msg/rle_indexed_builder
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
add_compile_fail_test(msg/fail/message_dup_fieldnames.cpp LIBRARIES warnings
                      cib)
