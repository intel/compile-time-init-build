cmake_minimum_required(VERSION 3.25)

project(
    cib
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION
        "A header-only C++ library for composing modular firmware at compile-time."
    HOMEPAGE_URL "https://github.com/intel/compile-time-init-build")

include(cmake/get_cpm.cmake)
if(PROJECT_IS_TOP_LEVEL)
    cpmaddpackage("gh:intel/cicd-repo-infrastructure#dev")
else()
    cpmaddpackage("gh:intel/cicd-repo-infrastructure#3e2bef0")
endif()

find_package(
    Python3
    COMPONENTS Interpreter
    REQUIRED)

include(cmake/string_catalog.cmake)

add_versioned_package("gh:boostorg/mp11#boost-1.83.0")
fmt_recipe(10.2.1)
add_versioned_package("gh:intel/cpp-baremetal-concurrency#7c5b26c")
add_versioned_package("gh:intel/cpp-std-extensions#2834680")
add_versioned_package("gh:intel/cpp-baremetal-senders-and-receivers#73d95bc")

set(GEN_STR_CATALOG
    ${CMAKE_CURRENT_LIST_DIR}/tools/gen_str_catalog.py
    CACHE FILEPATH "Location of script to generate a string catalog")

add_library(cib_sc INTERFACE)
target_compile_features(cib_sc INTERFACE cxx_std_20)
target_link_libraries_system(cib_sc INTERFACE fmt::fmt-header-only stdx)

target_sources(
    cib_sc
    INTERFACE FILE_SET
              sc
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/sc/format.hpp
              include/sc/fwd.hpp
              include/sc/lazy_string_format.hpp
              include/sc/string_constant.hpp)

add_library(cib_match INTERFACE)
target_compile_features(cib_match INTERFACE cxx_std_20)
target_link_libraries_system(cib_match INTERFACE cib_sc stdx)

target_sources(
    cib_match
    INTERFACE FILE_SET
              match
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/match/and.hpp
              include/match/bin_op.hpp
              include/match/concepts.hpp
              include/match/constant.hpp
              include/match/cost.hpp
              include/match/implies.hpp
              include/match/negate.hpp
              include/match/not.hpp
              include/match/ops.hpp
              include/match/or.hpp
              include/match/predicate.hpp
              include/match/simplify.hpp
              include/match/sum_of_products.hpp)

add_library(cib_interrupt INTERFACE)
target_compile_features(cib_interrupt INTERFACE cxx_std_20)
target_link_libraries_system(cib_interrupt INTERFACE cib_sc concurrency stdx)

target_sources(
    cib_interrupt
    INTERFACE FILE_SET
              interrupt
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/interrupt/concepts.hpp
              include/interrupt/config.hpp
              include/interrupt/dynamic_controller.hpp
              include/interrupt/fwd.hpp
              include/interrupt/hal.hpp
              include/interrupt/impl.hpp
              include/interrupt/manager.hpp
              include/interrupt/policies.hpp)

add_library(cib_lookup INTERFACE)
target_compile_features(cib_lookup INTERFACE cxx_std_20)
target_link_libraries_system(cib_lookup INTERFACE stdx)

target_sources(
    cib_lookup
    INTERFACE FILE_SET
              lookup
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/lookup/detail/select.hpp
              include/lookup/entry.hpp
              include/lookup/input.hpp
              include/lookup/linear_search_lookup.hpp
              include/lookup/lookup.hpp
              include/lookup/pseudo_pext_lookup.hpp
              include/lookup/strategies.hpp
              include/lookup/strategy_failed.hpp)

add_library(cib_log INTERFACE)
target_compile_features(cib_log INTERFACE cxx_std_20)
target_link_libraries_system(cib_log INTERFACE cib_sc stdx)

target_sources(
    cib_log
    INTERFACE FILE_SET
              log
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/log/env.hpp
              include/log/level.hpp
              include/log/log.hpp
              include/log/module.hpp)

add_library(cib_msg INTERFACE)
target_compile_features(cib_msg INTERFACE cxx_std_20)
target_link_libraries_system(
    cib_msg
    INTERFACE
    async
    cib_log
    cib_lookup
    cib_match
    cib_sc
    stdx)

target_sources(
    cib_msg
    INTERFACE FILE_SET
              msg
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/msg/callback.hpp
              include/msg/detail/indexed_builder_common.hpp
              include/msg/detail/indexed_handler_common.hpp
              include/msg/detail/separate_sum_terms.hpp
              include/msg/field.hpp
              include/msg/field_matchers.hpp
              include/msg/handler_builder.hpp
              include/msg/handler.hpp
              include/msg/handler_interface.hpp
              include/msg/indexed_builder.hpp
              include/msg/indexed_handler.hpp
              include/msg/indexed_service.hpp
              include/msg/message.hpp
              include/msg/send.hpp
              include/msg/service.hpp)

add_library(cib_log_fmt INTERFACE)
target_compile_features(cib_log_fmt INTERFACE cxx_std_20)
target_link_libraries_system(cib_log_fmt INTERFACE cib_log fmt::fmt-header-only
                             stdx)

target_sources(
    cib_log_fmt
    INTERFACE FILE_SET
              log
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/log/fmt/logger.hpp)

add_library(cib_log_mipi INTERFACE)
target_compile_features(cib_log_mipi INTERFACE cxx_std_20)
target_link_libraries_system(cib_log_mipi INTERFACE cib_log cib_msg concurrency
                             stdx)

target_sources(
    cib_log_mipi
    INTERFACE FILE_SET
              log
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/log/catalog/catalog.hpp
              include/log/catalog/mipi_builder.hpp
              include/log/catalog/mipi_encoder.hpp
              include/log/catalog/mipi_messages.hpp)

add_library(cib_nexus INTERFACE)
target_compile_features(cib_nexus INTERFACE cxx_std_20)
target_link_libraries_system(cib_nexus INTERFACE stdx)

target_sources(
    cib_nexus
    INTERFACE FILE_SET
              nexus
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/cib/builder_meta.hpp
              include/cib/built.hpp
              include/cib/callback.hpp
              include/cib/config.hpp
              include/cib/detail/components.hpp
              include/cib/detail/constexpr_conditional.hpp
              include/cib/detail/runtime_conditional.hpp
              include/cib/detail/config_details.hpp
              include/cib/detail/config_item.hpp
              include/cib/detail/exports.hpp
              include/cib/detail/extend.hpp
              include/cib/detail/nexus_details.hpp
              include/cib/func_decl.hpp
              include/cib/nexus.hpp)

add_library(cib_flow INTERFACE)
target_compile_features(cib_flow INTERFACE cxx_std_20)
target_link_libraries_system(cib_flow INTERFACE cib_log cib_nexus cib_sc stdx)

target_sources(
    cib_flow
    INTERFACE FILE_SET
              flow
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/flow/builder.hpp
              include/flow/common.hpp
              include/flow/detail/par.hpp
              include/flow/detail/seq.hpp
              include/flow/detail/walk.hpp
              include/flow/flow.hpp
              include/flow/graph_builder.hpp
              include/flow/graphviz_builder.hpp
              include/flow/impl.hpp
              include/flow/run.hpp
              include/flow/step.hpp)

add_library(cib_seq INTERFACE)
target_compile_features(cib_seq INTERFACE cxx_std_20)
target_link_libraries_system(
    cib_seq
    INTERFACE
    cib_flow
    cib_log
    cib_nexus
    cib_sc
    stdx)

target_sources(
    cib_seq
    INTERFACE FILE_SET
              seq
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/seq/builder.hpp
              include/seq/impl.hpp
              include/seq/step.hpp)

add_library(cib INTERFACE)
target_compile_features(cib INTERFACE cxx_std_20)
target_link_libraries_system(
    cib
    INTERFACE
    async
    cib_flow
    cib_interrupt
    cib_log
    cib_log_fmt
    cib_log_mipi
    cib_lookup
    cib_match
    cib_msg
    cib_nexus
    cib_sc
    cib_seq
    concurrency
    fmt::fmt-header-only
    stdx)

target_sources(
    cib
    INTERFACE FILE_SET
              cib
              TYPE
              HEADERS
              BASE_DIRS
              include
              FILES
              include/cib/cib.hpp
              include/cib/top.hpp)

if(PROJECT_IS_TOP_LEVEL)
    add_docs(docs)
    clang_tidy_interface(cib)
    clang_tidy_interface(cib_flow)
    clang_tidy_interface(cib_interrupt)
    clang_tidy_interface(cib_lookup)
    clang_tidy_interface(cib_log)
    clang_tidy_interface(cib_log_fmt)
    clang_tidy_interface(cib_log_mipi)
    clang_tidy_interface(cib_match)
    clang_tidy_interface(cib_msg)
    clang_tidy_interface(cib_nexus)
    clang_tidy_interface(cib_sc)
    clang_tidy_interface(cib_seq)

    # Enable functional and performance test suites.
    add_subdirectory(test)
    add_subdirectory(benchmark)
    add_subdirectory(examples)
    add_subdirectory(tools)

    # Build single-header release.
    include(cmake/single_header.cmake)
    gen_single_header(
        TARGET
        release_header
        GEN_HEADER
        ${CMAKE_SOURCE_DIR}/tools/gen_release_header.py
        SOURCE_TARGET
        cib
        SOURCE_FILESET
        cib
        INPUT_HEADER
        ${CMAKE_SOURCE_DIR}/include/cib/cib.hpp
        OUTPUT_HEADER
        ${CMAKE_BINARY_DIR}/include/cib/cib.hpp)

    if(DEFINED ENV{SINGLE_HEADER})
        add_dependencies(cib release_header)

        target_include_directories(
            cib
            INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
    else()
        target_include_directories(
            cib
            INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>)
    endif()
endif()
