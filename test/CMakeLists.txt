add_unit_test(
    cib_test
    CATCH2
    FILES
    cib/builder_meta.cpp
    cib/callback.cpp
    cib/nexus.cpp
    cib/readme_hello_world.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    container_test
    CATCH2
    FILES
    container/constexpr_map.cpp
    container/constexpr_multimap.cpp
    container/constexpr_set.cpp
    container/vector.cpp
    container/queue.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    flow_test
    CATCH2
    FILES
    flow/flow.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    interrupt_test
    CATCH2
    FILES
    interrupt/dynamic_controller.cpp
    interrupt/policies.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    interrupt_manager_test
    GTEST
    FILES
    interrupt/manager.cpp
    INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/test/
    LIBRARIES
    warnings
    cib)

add_unit_test(
    log_test
    CATCH2
    FILES
    log/log.cpp
    INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/test/
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
    INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/test/
    LIBRARIES
    warnings
    catalog_lib
    catalog_strings)

add_unit_test(
    log_fmt_test
    CATCH2
    FILES
    log/fmt_logger.cpp
    INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/test/
    LIBRARIES
    warnings
    cib)

add_unit_test(
    log_mipi_test
    CATCH2
    FILES
    log/mipi_encoder.cpp
    INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/test/
    LIBRARIES
    warnings
    cib)

add_unit_test(
    msg_test
    CATCH2
    FILES
    msg/match.cpp
    msg/detail/bitset.cpp
    msg/disjoint_field.cpp
    msg/field.cpp
    msg/handler.cpp
    msg/handler_builder.cpp
    msg/indexed_handler.cpp
    msg/indexed_builder.cpp
    msg/message.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    lookup_test
    CATCH2
    FILES
    lookup/lookup.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    sc_test
    CATCH2
    FILES
    sc/format.cpp
    sc/string_constant.cpp
    LIBRARIES
    warnings
    cib)

add_unit_test(
    seq_test
    CATCH2
    FILES
    seq/sequencer.cpp
    LIBRARIES
    warnings
    cib)
