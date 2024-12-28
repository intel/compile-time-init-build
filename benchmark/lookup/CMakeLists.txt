add_versioned_package("gh:boost-ext/mph#v1.0.8")
add_versioned_package("gh:serge-sans-paille/frozen#292a811")

# Define the variable values
set(BENCH_ALG_NAMES
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

foreach(type IN ITEMS uint16 uint32)
    foreach(i RANGE 1 10)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
    foreach(i RANGE 20 100 10)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
    foreach(i RANGE 200 1000 100)
        list(APPEND BENCH_DATASETS exp_${i}_${type})
    endforeach()
endforeach()

set(EXCLUDED_COMBINATIONS
    mph_pext_exp_70_uint32
    mph_pext_exp_80_uint32
    mph_pext_exp_90_uint32
    mph_pext_exp_100_uint32
    mph_pext_exp_200_uint32
    mph_pext_exp_300_uint32
    mph_pext_exp_400_uint32
    mph_pext_exp_500_uint32
    mph_pext_exp_600_uint32
    mph_pext_exp_700_uint32
    mph_pext_exp_800_uint32
    mph_pext_exp_900_uint32
    mph_pext_exp_1000_uint32
    mph_pext_exp_70_uint16
    mph_pext_exp_80_uint16
    mph_pext_exp_90_uint16
    mph_pext_exp_100_uint16
    mph_pext_exp_200_uint16
    mph_pext_exp_300_uint16
    mph_pext_exp_400_uint16
    mph_pext_exp_500_uint16
    mph_pext_exp_600_uint16
    mph_pext_exp_700_uint16
    mph_pext_exp_800_uint16
    mph_pext_exp_900_uint16
    mph_pext_exp_1000_uint16)

# Generate all possible combinations of the variables
foreach(BENCH_ALG_NAME ${BENCH_ALG_NAMES})
    foreach(BENCH_DATASET ${BENCH_DATASETS})
        if("${BENCH_ALG_NAME}_${BENCH_DATASET}" IN_LIST EXCLUDED_COMBINATIONS)
            continue()
        endif()
        set(name "${BENCH_ALG_NAME}_${BENCH_DATASET}_bench")

        add_benchmark(
            ${name}
            NANO
            FILES
            pseudo_pext.cpp
            SYSTEM_LIBRARIES
            cib_lookup
            mph
            frozen-headers)
        target_compile_options(${name} PRIVATE -fconstexpr-steps=4000000000)
        target_compile_definitions(
            ${name}
            PRIVATE ALG_NAME=bench_${BENCH_ALG_NAME}
                    BENCH_DATASET=${BENCH_DATASET} ANKERL_NANOBENCH_IMPLEMENT)
    endforeach()
endforeach()
