add_tests(fmt_logger log mipi_encoder mipi_logger module_id)

add_library(catalog1_lib STATIC catalog1_lib.cpp)
add_library(catalog2_lib OBJECT catalog2a_lib.cpp catalog2b_lib.cpp)
target_include_directories(catalog1_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(catalog2_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
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
    catalog_extra.json
    STABLE_JSON
    stable_strings.json
    INPUT_HEADERS
    catalog_enums.hpp
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
    catalog_app.cpp
    LIBRARIES
    warnings
    cib
    catalog1_lib
    catalog2_lib
    catalog_strings)
