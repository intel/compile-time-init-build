add_library(warnings INTERFACE)

target_compile_options(
    warnings
    INTERFACE
        # warnings turned on
        -Wall
        $<$<CXX_COMPILER_ID:Clang>:-Warray-bounds-pointer-arithmetic>
        -Wcast-align
        -Wconversion
        -Wdouble-promotion
        $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
        $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
        -Werror
        -Wextra
        -Wextra-semi
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},8>>:-Wextra-semi-stmt>
        -Wfatal-errors
        -Wformat=2
        $<$<CXX_COMPILER_ID:Clang>:-Wgcc-compat>
        $<$<CXX_COMPILER_ID:Clang>:-Wheader-hygiene>
        $<$<CXX_COMPILER_ID:Clang>:-Widiomatic-parentheses>
        $<$<CXX_COMPILER_ID:Clang>:-Wimplicit>
        $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
        $<$<CXX_COMPILER_ID:Clang>:-Wnewline-eof>
        -Wold-style-cast
        -Woverloaded-virtual
        -Wshadow
        $<$<CXX_COMPILER_ID:GNU>:-Wuseless-cast>
        -Wunused
        $<$<CXX_COMPILER_ID:Clang>:-Wmissing-prototypes>
        # warnings turned off
        $<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-string-literal-operator-template>
        # other compilation flags
        $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=8>
        $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=8>
        -ftemplate-backtrace-limit=0
        $<$<STREQUAL:${CMAKE_GENERATOR},Ninja>:-fdiagnostics-color>)
