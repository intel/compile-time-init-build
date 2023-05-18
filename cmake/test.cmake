option(BUILD_TESTING "" OFF)
include(CTest)
add_custom_target(unit_tests)
add_custom_target(build_unit_tests)
set(CMAKE_TESTING_ENABLED
    1
    CACHE INTERNAL "")

find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set(MEMORYCHECK_SUPPRESSIONS_FILE
    "${CMAKE_CURRENT_LIST_DIR}/default.supp"
    CACHE FILEPATH "File that contains suppressions for the memory checker")
configure_file(${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
               ${PROJECT_BINARY_DIR}/DartConfiguration.tcl)

if(DEFINED ENV{CXX_STANDARD})
    set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

macro(get_catch2)
    if(NOT DEFINED CPM_GOT_CATCH)
        add_versioned_package("gh:catchorg/Catch2@3.3.2")
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
        include(Catch)
        set(CPM_GOT_CATCH 1)
    endif()
endmacro()

macro(get_gtest)
    if(NOT DEFINED CPM_GOT_GTEST)
        add_versioned_package("gh:google/googletest@1.13.0")
        include(GoogleTest)
        set(CPM_GOT_GTEST 1)
    endif()
endmacro()

macro(get_gunit)
    get_gtest()
    if(NOT DEFINED CPM_GOT_GUNIT)
        add_versioned_package(
            NAME
            gunit
            GIT_TAG
            d02ec96
            GITHUB_REPOSITORY
            cpp-testing/GUnit
            DOWNLOAD_ONLY
            YES)
        set(CPM_GOT_GUNIT 1)
    endif()
endmacro()

macro(add_boost_di)
    if(NOT DEFINED CPM_GOT_BOOST_DI)
        add_versioned_package("gh:boost-ext/di#9866a4a")
        set(CPM_GOT_BOOST_DI 1)
    endif()
endmacro()

macro(add_gherkin)
    if(NOT DEFINED CPM_GOT_GHERKIN)
        add_subdirectory(
            ${gunit_SOURCE_DIR}/libs/gherkin-cpp
            ${gunit_BINARY_DIR}/libs/gherkin-cpp EXCLUDE_FROM_ALL SYSTEM)
        set(CPM_GOT_GHERKIN 1)
    endif()
endmacro()

function(add_unit_test name)
    set(options CATCH2 GTEST GUNIT)
    set(multiValueArgs FILES INCLUDE_DIRECTORIES LIBRARIES SYSTEM_LIBRARIES)
    cmake_parse_arguments(UNIT "${options}" "" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${UNIT_FILES})
    target_include_directories(${name} PRIVATE ${UNIT_INCLUDE_DIRECTORIES})
    target_link_libraries(${name} PRIVATE ${UNIT_LIBRARIES})
    target_link_libraries_system(${name} PRIVATE ${UNIT_SYSTEM_LIBRARIES})
    target_link_libraries(${name} PRIVATE sanitizers)
    add_dependencies(build_unit_tests ${name})

    if(UNIT_CATCH2)
        get_catch2()
        catch_discover_tests(${name})
        target_link_libraries_system(${name} PRIVATE Catch2::Catch2WithMain)
        set(target_test_command $<TARGET_FILE:${name}> "--order" "rand")
    elseif(UNIT_GTEST)
        get_gtest()
        gtest_discover_tests(${name})
        target_link_libraries_system(${name} PRIVATE gmock gtest gmock_main)
        set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")
    elseif(UNIT_GUNIT)
        get_gunit()
        add_boost_di()
        gtest_discover_tests(${name})
        target_include_directories(
            ${name} SYSTEM
            PRIVATE ${gunit_SOURCE_DIR}/include
                    ${gunit_SOURCE_DIR}/libs/json/single_include/nlohmann)
        target_link_libraries_system(${name} PRIVATE gtest_main gmock_main
                                     Boost.DI)
        set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")
    else()
        set(target_test_command $<TARGET_FILE:${name}>)
        add_test(NAME ${name} COMMAND ${target_test_command})
    endif()

    add_custom_target(all_${name} ALL DEPENDS run_${name})
    add_custom_target(run_${name} DEPENDS ${name}.passed)
    add_custom_command(
        OUTPUT ${name}.passed
        COMMAND ${target_test_command}
        COMMAND ${CMAKE_COMMAND} "-E" "touch" "${name}.passed"
        DEPENDS ${name})

    add_dependencies(unit_tests "run_${name}")
endfunction()

function(add_feature_test name)
    set(singleValueArgs FEATURE)
    set(multiValueArgs FILES INCLUDE_DIRECTORIES LIBRARIES SYSTEM_LIBRARIES)
    cmake_parse_arguments(FEAT "${options}" "${singleValueArgs}"
                          "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${FEAT_FILES})
    target_include_directories(${name} PRIVATE ${FEAT_INCLUDE_DIRECTORIES})
    target_link_libraries(${name} PRIVATE ${FEAT_LIBRARIES})
    target_link_libraries_system(${name} PRIVATE ${FEAT_SYSTEM_LIBRARIES})
    target_link_libraries(${name} PRIVATE sanitizers)
    add_dependencies(build_unit_tests ${name})

    get_gunit()
    add_gherkin()
    add_boost_di()
    target_include_directories(
        ${name} SYSTEM
        PRIVATE ${gunit_SOURCE_DIR}/include
                ${gunit_SOURCE_DIR}/libs/json/single_include/nlohmann)
    target_link_libraries_system(${name} PRIVATE gtest_main gmock_main
                                 gherkin-cpp Boost.DI)
    set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")

    add_custom_target(all_${name} ALL DEPENDS run_${name})
    add_custom_target(run_${name} DEPENDS ${name}.passed ${FEAT_FEATURE})
    get_filename_component(FEATURE_FILE ${FEAT_FEATURE} ABSOLUTE)
    add_custom_command(
        OUTPUT ${name}.passed
        COMMAND ${CMAKE_COMMAND} -E env SCENARIO="${FEATURE_FILE}"
                ${target_test_command}
        COMMAND ${CMAKE_COMMAND} "-E" "touch" "${name}.passed"
        DEPENDS ${name})

    add_test(NAME ${name} COMMAND ${target_test_command})
    set_property(TEST ${name} PROPERTY ENVIRONMENT "SCENARIO=${FEATURE_FILE}")
    add_dependencies(unit_tests "run_${name}")
endfunction()
