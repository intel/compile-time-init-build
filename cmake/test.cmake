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
    if(NOT TARGET Catch2::Catch2WithMain)
        add_versioned_package("gh:catchorg/Catch2@3.4.0")
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
        include(Catch)
    endif()
endmacro()

macro(get_gtest)
    if(NOT TARGET gtest)
        add_versioned_package("gh:google/googletest@1.14.0")
        include(GoogleTest)
    endif()
endmacro()

macro(get_gunit)
    get_gtest()
    if(NOT DEFINED gunit_SOURCE_DIR)
        add_versioned_package(
            NAME
            gunit
            GIT_TAG
            d02ec96
            GITHUB_REPOSITORY
            cpp-testing/GUnit
            DOWNLOAD_ONLY
            YES)
    endif()
endmacro()

macro(add_boost_di)
    if(NOT TARGET Boost.DI)
        add_versioned_package("gh:boost-ext/di#9866a4a")
    endif()
endmacro()

macro(add_gherkin)
    if(NOT TARGET gherkin-cpp)
        add_subdirectory(
            ${gunit_SOURCE_DIR}/libs/gherkin-cpp
            ${gunit_BINARY_DIR}/libs/gherkin-cpp EXCLUDE_FROM_ALL SYSTEM)
    endif()
endmacro()

macro(add_rapidcheck)
    if(NOT TARGET rapidcheck)
        add_versioned_package(NAME rapidcheck GIT_TAG a5724ea GITHUB_REPOSITORY
                              emil-e/rapidcheck)
        add_subdirectory(
            ${rapidcheck_SOURCE_DIR}/extras/catch
            ${rapidcheck_BINARY_DIR}/extras/catch EXCLUDE_FROM_ALL SYSTEM)
        add_subdirectory(
            ${rapidcheck_SOURCE_DIR}/extras/gtest
            ${rapidcheck_BINARY_DIR}/extras/gtest EXCLUDE_FROM_ALL SYSTEM)
        add_subdirectory(
            ${rapidcheck_SOURCE_DIR}/extras/gmock
            ${rapidcheck_BINARY_DIR}/extras/gmock EXCLUDE_FROM_ALL SYSTEM)
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
        add_rapidcheck()
        catch_discover_tests(${name})
        target_link_libraries_system(${name} PRIVATE Catch2::Catch2WithMain
                                     rapidcheck rapidcheck_catch)
        set(target_test_command $<TARGET_FILE:${name}> "--order" "rand")
    elseif(UNIT_GTEST)
        get_gtest()
        add_rapidcheck()
        gtest_discover_tests(${name})
        target_link_libraries_system(
            ${name}
            PRIVATE
            gmock
            gtest
            gmock_main
            rapidcheck
            rapidcheck_gtest
            rapidcheck_gmock)
        set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")
    elseif(UNIT_GUNIT)
        get_gunit()
        add_boost_di()
        add_rapidcheck()
        target_include_directories(
            ${name} SYSTEM
            PRIVATE ${gunit_SOURCE_DIR}/include
                    ${gunit_SOURCE_DIR}/libs/json/single_include/nlohmann)
        target_link_libraries_system(
            ${name}
            PRIVATE
            gtest_main
            gmock_main
            Boost.DI
            rapidcheck
            rapidcheck_gtest
            rapidcheck_gmock)
        set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")
        add_test(NAME ${name} COMMAND ${target_test_command})
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
    add_rapidcheck()
    target_include_directories(
        ${name} SYSTEM
        PRIVATE ${gunit_SOURCE_DIR}/include
                ${gunit_SOURCE_DIR}/libs/json/single_include/nlohmann
                ${rapidcheck_SOURCE_DIR}/extras/gmock/include)
    target_link_libraries_system(
        ${name}
        PRIVATE
        gtest_main
        gmock_main
        gherkin-cpp
        Boost.DI
        rapidcheck
        rapidcheck_gtest
        rapidcheck_gmock)
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
