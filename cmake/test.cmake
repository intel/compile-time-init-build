enable_testing()
add_custom_target(unit_tests)

if(DEFINED ENV{CXX_STANDARD} AND NOT $ENV{CXX_STANDARD} EQUAL "")
    set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

add_versioned_package("gh:catchorg/Catch2@3.1.1")
add_versioned_package("gh:google/googletest#release-1.12.1")

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
include(GoogleTest)

function(add_unit_test name)
    set(options CATCH2 GTEST)
    set(multiValueArgs FILES INCLUDE_DIRECTORIES LIBRARIES SYSTEM_LIBRARIES)
    cmake_parse_arguments(UNIT "${options}" "" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${UNIT_FILES})
    target_include_directories(${name} PRIVATE ${UNIT_INCLUDE_DIRECTORIES})
    target_link_libraries(${name} PRIVATE ${UNIT_LIBRARIES})
    target_link_libraries_system(${name} PRIVATE ${UNIT_SYSTEM_LIBRARIES})
    target_link_libraries(${name} PRIVATE sanitizers)

    if(UNIT_CATCH2)
        catch_discover_tests(${name})
        target_link_libraries_system(${name} PRIVATE Catch2::Catch2WithMain)
        set(target_test_command $<TARGET_FILE:${name}> "--order" "rand")
    elseif(UNIT_GTEST)
        gtest_discover_tests(${name})
        target_link_libraries_system(${name} PRIVATE gmock gtest gmock_main)
        set(target_test_command $<TARGET_FILE:${name}> "--gtest_shuffle")
    else()
        add_test(NAME ${name} COMMAND ${target_test_command})
        set(target_test_command $<TARGET_FILE:${name}>)
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
