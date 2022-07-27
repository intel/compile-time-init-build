cmake_minimum_required(VERSION 3.16)
project(compile_time_init_build)


add_executable(tests
    detail/type_list.cpp
    detail/type_pack_element.cpp
    detail/type_to_string_view.cpp
    builder_meta.cpp
    callback.cpp
    nexus.cpp
    tuple.cpp
    set.cpp
    readme_hello_world.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tests
        PRIVATE
            -ferror-limit=8
    )

    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

    if (${CLANG_VERSION_STRING} VERSION_EQUAL 12)
        target_compile_options(tests
            PRIVATE
                -Wmisleading-indentation
                -Wno-c++20-compat
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
                -Wextra-semi
                -Wextra-semi-stmt

                -Wno-missing-prototypes

                # this complains about many cases in which ctad is supported and intended
                -Wno-ctad-maybe-unsupported

                -Wpedantic
                -Wall
                -Wextra
                -Wformat-security
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
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests
        PRIVATE
            -fmax-errors=8
    )

endif()

target_compile_options(tests
    PRIVATE
        -ftemplate-backtrace-limit=0
)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
include(${CMAKE_BINARY_DIR}/Catch2Config.cmake)

target_include_directories(tests PUBLIC ${Catch2_INCLUDE_DIRS})
target_link_libraries(tests Catch2::Catch2WithMain cib)

include(CTest)
include(Catch)

catch_discover_tests(tests)
