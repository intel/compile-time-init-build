#include "algorithms/pseudo_pext.hpp"

#include "algorithms/frozen_map.hpp"
#include "algorithms/frozen_unordered_map.hpp"
#include "algorithms/std_map.hpp"
#include "algorithms/std_unordered_map.hpp"

#include <cstdio>

#define STRINGIFY(S) #S
#define STR(S) STRINGIFY(S)

int main() {
    printf("\n\n\ndataset:   %s\n", STR(DATASET));
    printf("algorithm: %s\n", STR(ALG_NAME));
    ALG_NAME<DATASET, decltype(DATASET[0].first)>(STR(DATASET));
}
