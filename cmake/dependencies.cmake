include(cmake/CPM.cmake)

if(NOT DEFINED ENV{CPM_SOURCE_CACHE})
    message(
        STATUS
            "The environment variable \$CPM_SOURCE_CACHE is not defined: defining a CPM cache is recommended to avoid extra downloads."
    )
endif()

function(check_version dep required comp)
    foreach(present ${ARGN})
        string(REGEX MATCH "[0-9]+(\.[0-9]+)*" req ${required})
        string(REGEX MATCH "[0-9]+(\.[0-9]+)*" pre ${present})
        string(REPLACE ${req} " " req_leftover ${required})
        string(REPLACE ${pre} " " pre_leftover ${present})
        if(${req_leftover} STREQUAL ${pre_leftover})
            if(pre VERSION_${comp} req)
                if(NOT pre VERSION_EQUAL req)
                    message(
                        STATUS
                            "Dependency: ${dep} required version ${version} (${comp}) is fulfilled by version ${present}."
                    )
                endif()
                return()
            endif()
        endif()
    endforeach()
    message(
        FATAL_ERROR
            "Dependency error for ${dep}: ${version} (${comp}) not fulfilled by one of: ${ARGN}."
    )
endfunction()

function(identify_git_tag dep tag)
    execute_process(
        COMMAND git tag -l ${tag}
        WORKING_DIRECTORY ${${dep}_SOURCE_DIR}
        OUTPUT_VARIABLE out_tag
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(out_tag STREQUAL tag)
        set(is_tag
            1
            PARENT_SCOPE)
    endif()
endfunction()

function(check_dependency_version dep version)
    if(${dep}_SOURCE_DIR)
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${${dep}_SOURCE_DIR}
            OUTPUT_VARIABLE head_hash
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(
            COMMAND git tag -l --points-at ${head_hash}
            WORKING_DIRECTORY ${${dep}_SOURCE_DIR}
            OUTPUT_VARIABLE head_version
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(head_version)
            string(REPLACE "\n" ";" head_version ${head_version})
        endif()

        set(comp EQUAL)
        if(${ARGC} GREATER 2)
            set(comp ${ARGV2})
        endif()

        identify_git_tag(${dep} ${version})
        if(is_tag)
            check_version(${dep} ${version} ${comp} ${head_version})
        else()
            execute_process(
                COMMAND git merge-base --is-ancestor ${version} ${head_hash}
                WORKING_DIRECTORY ${${dep}_SOURCE_DIR}
                RESULT_VARIABLE hash_ok)
            if(hash_ok EQUAL 0)
                if(NOT head_hash STREQUAL version)
                    message(
                        STATUS
                            "Dependency: ${dep} required version ${version} is fulfilled by version ${head_hash}."
                    )
                endif()
            else()
                if(head_version)
                    check_version(${dep} ${version} ${comp} ${head_version})
                else()
                    message(
                        FATAL_ERROR
                            "Dependency error for ${dep}: ${version} is not an ancestor of ${head_hash}."
                    )
                endif()
            endif()
        endif()
    else()
        message(
            FATAL_ERROR
                "Missing required dependency: ${dep} at version ${version}.")
    endif()
endfunction()

function(add_versioned_package)
    cpmaddpackage("${ARGN}")

    list(LENGTH ARGN argnLength)
    if(argnLength EQUAL 1)
        cpm_parse_add_package_single_arg("${ARGN}" ARGN)
    endif()

    set(oneValueArgs VERSION GIT_TAG COMPARE)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" "${ARGN}")
    if(NOT DEFINED ARGS_GIT_TAG)
        set(ARGS_GIT_TAG v${ARGS_VERSION})
    endif()
    if(NOT DEFINED ARGS_COMPARE)
        set(ARGS_COMPARE GREATER_EQUAL)
    endif()

    check_dependency_version(${CPM_LAST_PACKAGE_NAME} ${ARGS_GIT_TAG}
                             ${ARGS_COMPARE})

    set(CPM_LAST_PACKAGE_NAME
        ${CPM_LAST_PACKAGE_NAME}
        PARENT_SCOPE)
    set(${CPM_LAST_PACKAGE_NAME}_SOURCE_DIR
        ${${CPM_LAST_PACKAGE_NAME}_SOURCE_DIR}
        PARENT_SCOPE)
    set(${CPM_LAST_PACKAGE_NAME}_BINARY_DIR
        ${${CPM_LAST_PACKAGE_NAME}_BINARY_DIR}
        PARENT_SCOPE)
    set(${CPM_LAST_PACKAGE_NAME}_ADDED
        ${${CPM_LAST_PACKAGE_NAME}_ADDED}
        PARENT_SCOPE)
endfunction()
