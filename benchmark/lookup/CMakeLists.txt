add_versioned_package("gh:boost-ext/mph#v1.0.8")
add_versioned_package("gh:serge-sans-paille/frozen#292a811")

function(gen_lookup_data)
    set(oneValueArgs TARGET TYPE SIZE OUTPUT)
    cmake_parse_arguments(GEN "" "${oneValueArgs}" "" ${ARGN})

    set(script "${CMAKE_SOURCE_DIR}/tools/benchmark/gen_map_data.py")
    get_filename_component(DIR "${GEN_OUTPUT}" DIRECTORY)

    add_custom_command(
        OUTPUT ${GEN_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIR}"
        COMMAND ${Python3_EXECUTABLE} ${script} --type ${GEN_TYPE} --size
                ${GEN_SIZE} --output ${GEN_OUTPUT}
        DEPENDS ${script}
        COMMAND_EXPAND_LISTS)
    add_custom_target(${GEN_TARGET} DEPENDS ${GEN_OUTPUT})
endfunction()

set(ALG_NAMES
    std_map
    std_unordered_map
    frozen_map
    frozen_unordered_map
    pseudo_pext_direct
    pseudo_pext_indirect_1
    pseudo_pext_indirect_2
    pseudo_pext_indirect_3
    pseudo_pext_indirect_4
    pseudo_pext_indirect_5
    pseudo_pext_indirect_6)

set(EXCLUDED_COMBINATIONS
    mph_pext_exp_uint32_70
    mph_pext_exp_uint32_80
    mph_pext_exp_uint32_90
    mph_pext_exp_uint32_100
    mph_pext_exp_uint32_200
    mph_pext_exp_uint32_300
    mph_pext_exp_uint32_400
    mph_pext_exp_uint32_500
    mph_pext_exp_uint32_600
    mph_pext_exp_uint32_700
    mph_pext_exp_uint32_800
    mph_pext_exp_uint32_900
    mph_pext_exp_uint32_1000
    mph_pext_exp_uint16_70
    mph_pext_exp_uint16_80
    mph_pext_exp_uint16_90
    mph_pext_exp_uint16_100
    mph_pext_exp_uint16_200
    mph_pext_exp_uint16_300
    mph_pext_exp_uint16_400
    mph_pext_exp_uint16_500
    mph_pext_exp_uint16_600
    mph_pext_exp_uint16_700
    mph_pext_exp_uint16_800
    mph_pext_exp_uint16_900
    mph_pext_exp_uint16_1000)

function(gen_pp_benchmarks)
    set(oneValueArgs TYPE SIZE)
    cmake_parse_arguments(BM "" "${oneValueArgs}" "" ${ARGN})

    set(DATASET exp_${BM_TYPE}_${BM_SIZE})
    set(HEADER "${CMAKE_BINARY_DIR}/benchmark/generated/${DATASET}.hpp")
    set(DATA_TARGET "bm_lookup_data_${DATASET}")
    gen_lookup_data(
        TARGET
        ${DATA_TARGET}
        TYPE
        ${BM_TYPE}
        SIZE
        ${BM_SIZE}
        OUTPUT
        ${HEADER})

    foreach(ALG_NAME ${ALG_NAMES})
        if("${ALG_NAME}_${DATASET}" IN_LIST EXCLUDED_COMBINATIONS)
            continue()
        endif()

        set(name "${ALG_NAME}_${DATASET}_bench")
        add_benchmark(
            ${name}
            NANO
            FILES
            pseudo_pext.cpp
            SYSTEM_LIBRARIES
            cib_lookup
            mph
            frozen-headers)
        target_compile_options(
            ${name}
            PRIVATE
                $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fconstexpr-steps=4000000000>
                $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=4000000000>
                --include=${HEADER})
        target_compile_definitions(
            ${name} PRIVATE ALG_NAME=bench_${ALG_NAME} DATASET=${DATASET}
                            ANKERL_NANOBENCH_IMPLEMENT)
        add_dependencies(${name} ${DATA_TARGET})
    endforeach()
endfunction()

foreach(type IN ITEMS uint16 uint32)
    foreach(i RANGE 1 10)
        gen_pp_benchmarks(TYPE ${type} SIZE ${i})
    endforeach()
    foreach(i RANGE 20 100 10)
        gen_pp_benchmarks(TYPE ${type} SIZE ${i})
    endforeach()
    foreach(i RANGE 200 1000 100)
        gen_pp_benchmarks(TYPE ${type} SIZE ${i})
    endforeach()
endforeach()
