cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

set(CMAKE_CXX_STANDARD 17)

add_executable(tests detail/meta.cpp builder_meta.cpp callback.cpp nexus.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tests
        PRIVATE
            -ferror-limit=2
            -Weverything
            -Wshadow-all
            -Warray-bounds-pointer-arithmetic
            -Wextra-semi-stmt
            -Wfor-loop-analysis
            -Wgcc-compat
            -Wglobal-constructors
            -Wgnu
            -Wheader-hygiene
            -Widiomatic-parentheses
            -Wnewline-eof
            -Wimplicit
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests
        PRIVATE
            -fmax-errors=2
            -Wshadow
    )
endif()


target_compile_options(tests
    PRIVATE
        -ftemplate-backtrace-limit=0
        -Werror
        -Wpedantic
        -Wall
        -Wextra
        -Wextra-semi
        -Wformat-security
        -Wextra-semi
        -Wfloat-conversion
        -Wframe-address
        -Wignored-qualifiers
        -Wold-style-cast
        -Wunused
        -Wmisleading-indentation
        -Weffc++

        # not feasible to change tuple padding for now
        -Wno-padded

        # clang complains about @example, which seems to be a valid tag
        -Wno-documentation-unknown-command

        # clang complains about @tparam on a template type alias
        -Wno-documentation

        # compatibility with anything less than C++17 is not a goal
        -Wno-c++98-compat-pedantic
        -Wno-c++98-compat
        -Wno-c++11-compat
        -Wno-c++14-compat

        # this complains about many cases in which ctad is supported and intended
        -Wno-ctad-maybe-unsupported
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain Cib)

# ensure the catch2 headers do not produce warnings
target_include_directories(tests SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib/Catch2/src/)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
