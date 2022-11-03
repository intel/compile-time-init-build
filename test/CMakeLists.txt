if(DEFINED ENV{CXX_STANDARD} AND NOT $ENV{CXX_STANDARD} EQUAL "")
    set(CMAKE_CXX_STANDARD $ENV{CXX_STANDARD})
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

add_versioned_package("gh:catchorg/Catch2@3.1.1")
add_versioned_package("gh:google/googletest#release-1.12.1")

# Catch tests
add_executable(
    cib-catch-tests
    cib/detail/type_list.cpp
    cib/detail/type_pack_element.cpp
    cib/detail/type_to_string_view.cpp
    cib/builder_meta.cpp
    cib/callback.cpp
    cib/nexus.cpp
    cib/tuple.cpp
    cib/set.cpp
    cib/readme_hello_world.cpp
    flow/flow.cpp
    sc/format.cpp
    sc/string_constant.cpp
    container/Array.cpp
    container/ConstexprMap.cpp
    container/ConstexprMultiMap.cpp
    container/ConstexprSet.cpp
    container/DoubleLinkedList.cpp
    container/LinkedList.cpp
    container/Vector.cpp
    container/PriorityQueue.cpp
    container/Queue.cpp
    msg/match.cpp
    msg/disjoint_field.cpp
    msg/field.cpp
    msg/handler.cpp
    msg/message.cpp)

target_link_libraries_system(cib-catch-tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(cib-catch-tests PRIVATE cib warnings)
target_include_directories(cib-catch-tests PRIVATE ${CMAKE_SOURCE_DIR}/test/)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(cib-catch-tests)

# Google tests
add_executable(cib-gtests interrupt/manager.cpp)

target_link_libraries_system(cib-gtests PRIVATE gmock gtest gmock_main)
target_link_libraries(cib-gtests PRIVATE cib warnings)
target_include_directories(cib-gtests PRIVATE ${CMAKE_SOURCE_DIR}/test/)

include(GoogleTest)
gtest_discover_tests(cib-gtests)

# Top-level test target
add_custom_target(all-cib-tests DEPENDS cib-gtests cib-catch-tests)
