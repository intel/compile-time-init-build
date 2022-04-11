cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)

set(CMAKE_CXX_STANDARD 17)

add_executable(tests
    detail/type_list.cpp
    detail/meta.cpp
    builder_meta.cpp
    callback.cpp
    nexus.cpp
    big_nexus.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tests
        PRIVATE
            -ferror-limit=8
            -Weverything
            -Wshadow-all
            -Warray-bounds-pointer-arithmetic

            -Wfor-loop-analysis
            -Wgcc-compat
            -Wglobal-constructors
            -Wgnu
            -Wheader-hygiene
            -Widiomatic-parentheses
            -Wnewline-eof
            -Wimplicit

            -Wno-missing-prototypes
    )

    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

    if (${CLANG_VERSION_STRING} VERSION_GREATER 9)
        target_compile_options(tests
            PRIVATE
                -Wmisleading-indentation
                -Wno-c++20-compat
        )
    endif()

    if (${CLANG_VERSION_STRING} VERSION_GREATER 8)
        target_compile_options(tests
            PRIVATE
                # this complains about many cases in which ctad is supported and intended
                -Wno-ctad-maybe-unsupported
                -ftime-trace
        )
    endif()

    if (${CLANG_VERSION_STRING} VERSION_GREATER 7)
        target_compile_options(tests
            PRIVATE
                -Wextra-semi-stmt
        )
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests
        PRIVATE
            -fmax-errors=8
            -Wshadow
            -Wmisleading-indentation

            # this complains about many cases in which ctad is supported and intended
            -Wno-ctad-maybe-unsupported
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
        -Wignored-qualifiers
        -Wold-style-cast
        -Wunused
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
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain Cib)

# ensure the catch2 headers do not produce warnings
target_include_directories(tests SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib/Catch2/src/)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
