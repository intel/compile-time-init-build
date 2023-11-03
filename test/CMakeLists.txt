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

add_tests(
    cib/builder_meta
    cib/callback
    cib/nexus
    cib/readme_hello_world
    flow/flow
    interrupt/dynamic_controller
    interrupt/policies
    log/fmt_logger
    log/log
    log/mipi_encoder
    lookup/direct_array
    lookup/fast_hash
    lookup/input
    lookup/linear_search
    lookup/lookup
    lookup/strategies
    match/and
    match/constant
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

add_unit_test(
    interrupt_manager_test
    GTEST
    FILES
    interrupt/manager.cpp
    LIBRARIES
    warnings
    cib)

add_library(catalog_lib log/catalog_lib.cpp)
target_include_directories(catalog_lib PRIVATE ${CMAKE_SOURCE_DIR}/test/)
target_link_libraries(catalog_lib PRIVATE warnings cib)
gen_str_catalog(
    GEN_STR_CATALOG
    ${CMAKE_SOURCE_DIR}/tools/gen_str_catalog.py
    OUTPUT_CPP
    ${CMAKE_CURRENT_BINARY_DIR}/strings.cpp
    OUTPUT_JSON
    ${CMAKE_CURRENT_BINARY_DIR}/strings.json
    OUTPUT_XML
    ${CMAKE_CURRENT_BINARY_DIR}/strings.xml
    OUTPUT_LIB
    catalog_strings
    INPUT_LIBS
    catalog_lib)

add_unit_test(
    log_catalog_test
    CATCH2
    FILES
    log/catalog_app.cpp
    LIBRARIES
    warnings
    catalog_lib
    catalog_strings)

add_compile_fail_test(msg/fail/impossible_match_callback.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/impossible_match_field.cpp LIBRARIES warnings
                      cib)
add_compile_fail_test(msg/fail/field_location.cpp LIBRARIES warnings cib)
add_compile_fail_test(msg/fail/field_size.cpp LIBRARIES warnings cib)
