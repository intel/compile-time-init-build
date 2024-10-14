add_executable(pbt_prototype_driver EXCLUDE_FROM_ALL pbt_prototype_driver.cpp)
target_link_libraries(pbt_prototype_driver PUBLIC sanitizers warnings cib)
target_compile_options(
    pbt_prototype_driver
    PUBLIC $<$<CXX_COMPILER_ID:Clang>:-fconstexpr-steps=4000000000>)

add_unit_test(
    lookup
    PYTEST
    FILES
    lookup.py
    EXTRA_ARGS
    -vv
    -n
    auto
    -x
    --compile-commands=${CMAKE_BINARY_DIR}/compile_commands.json
    --prototype-driver=${CMAKE_CURRENT_SOURCE_DIR}/pbt_prototype_driver.cpp)
