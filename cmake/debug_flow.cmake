if(COMMAND generate_flow_graph)
    return()
endif()

function(generate_flow_graph)
    cmake_parse_arguments(FG "" "TARGET" "" ${ARGN})
    message(
        STATUS
            "generate_flow_graph(${FG_TARGET}) is disabled because mmdc was not found."
    )
endfunction()

find_program(MMDC_PROGRAM "mmdc")
if(MMDC_PROGRAM)
    message(STATUS "mmdc found at ${MMDC_PROGRAM}.")
else()
    message(STATUS "mmdc not found. generate_flow_graph will not be available.")
    return()
endif()

function(copy_target_property OLD NEW PROP)
    get_target_property(val ${OLD} ${PROP})
    if(NOT "${val}" STREQUAL "val-NOTFOUND")
        set_target_properties(${NEW} PROPERTIES "${PROP}" "${val}")
    endif()
endfunction(copy_target_property)

function(copy_target_properties OLD NEW)
    foreach(prop ${ARGN})
        copy_target_property(${OLD} ${NEW} ${prop})
        copy_target_property(${OLD} ${NEW} INTERFACE_${prop})
    endforeach(prop)
endfunction(copy_target_properties)

function(make_flow_debug_target OLD NEW)
    add_library(${NEW} STATIC EXCLUDE_FROM_ALL
                $<TARGET_PROPERTY:${FG_TARGET},SOURCES>)
    copy_target_properties(
        ${OLD}
        ${NEW}
        COMPILE_DEFINITIONS
        COMPILE_FEATURES
        COMPILE_OPTIONS
        INCLUDE_DIRECTORIES
        LINK_LIBRARIES
        PRECOMPILE_HEADERS)
endfunction()

function(generate_flow_graph)
    set(oneValueArgs TARGET FLOW_NAME START_NODE END_NODE)
    cmake_parse_arguments(FG "" "${oneValueArgs}" "" ${ARGN})
    set(target_stem "${FG_TARGET}.${FG_FLOW_NAME}")

    # generate a header that overrides the flow service
    get_filename_component(header "${target_stem}.debug.hpp" ABSOLUTE BASE_DIR
                           ${CMAKE_CURRENT_BINARY_DIR})
    set(FLOW_NAME ${FG_FLOW_NAME})
    set(START_NODE ${FG_START_NODE})
    set(END_NODE ${FG_END_NODE})
    set(RENDERER "mermaid")
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/debug_flow.hpp.in"
                   ${header} @ONLY)

    # set up a library that injects the header
    set(injection_lib "${target_stem}.lib")
    add_library(${injection_lib} INTERFACE)
    target_compile_options(${injection_lib} INTERFACE -include ${header})

    # set up a new target from the existing one
    set(debug_target "${target_stem}.bin")
    make_flow_debug_target(${FG_TARGET} ${debug_target})
    target_link_libraries(${debug_target} PRIVATE ${injection_lib})

    # generate the graph source
    set(graph_stem "${target_stem}.graph")
    set(graph_source "${graph_stem}.mmd")
    set(graph "${graph_stem}.svg")
    set(awk_gcc_cmd "awk '{gsub(\"\\\\\\\\012\",\"\\n\")}\;1'")
    set(awk_clang_cmd "awk '{gsub(\"\\\\\\\\n\",\"\\n\")}\;1'")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${graph_source}
        DEPENDS $<TARGET_PROPERTY:${debug_target},SOURCES>
        DEPENDS ${header}
        COMMAND
            ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
            ${debug_target} | bash -c ${awk_gcc_cmd} | bash -c ${awk_clang_cmd}
            | tac | sed -n "/^~~~~END/,/^~~~~START/{p;/^~~~~START/q}" | tac |
            tail -n +2 | head -n -1 >
            "${CMAKE_CURRENT_BINARY_DIR}/${graph_source}"
        VERBATIM)

    # build the graph from the source
    if(EXISTS "${CMAKE_SOURCE_DIR}/docs/mermaid.conf")
        set(mm_conf "-c;${CMAKE_SOURCE_DIR}/docs/mermaid.conf")
    endif()
    if(EXISTS "${CMAKE_SOURCE_DIR}/docs/puppeteer_config.json")
        set(puppeteer_conf "-p;${CMAKE_SOURCE_DIR}/docs/puppeteer_config.json")
    endif()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${graph}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${graph_source}
        COMMAND
            ${MMDC_PROGRAM} -i "${CMAKE_CURRENT_BINARY_DIR}/${graph_source}" -o
            "${CMAKE_CURRENT_BINARY_DIR}/${graph}" ${mm_conf} ${puppeteer_conf}
        COMMAND_EXPAND_LISTS)
    add_custom_target("${graph_stem}"
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${graph})
endfunction()
