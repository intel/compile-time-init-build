#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_QUICKSORT_HPP
#define COMPILE_TIME_INIT_BUILD_QUICKSORT_HPP


namespace cib::detail {
    // https://en.wikipedia.org/wiki/Quicksort#Hoare_partition_scheme

    template<typename T>
    CIB_CONSTEVAL static std::size_t partition(T * elems, std::size_t lo, std::size_t hi) {
        auto const pivot = elems[(hi + lo) / 2];

        auto i = lo - 1;
        auto j = hi + 1;

        while (true) {
            do {i = i + 1;} while (elems[i] < pivot);
            do {j = j - 1;} while (elems[j] > pivot);
            if (i >= j) {return j;}

            auto const temp = elems[i];
            elems[i] = elems[j];
            elems[j] = temp;
        }
    }

    template<typename T>
    CIB_CONSTEVAL static void quicksort(T * elems, std::size_t lo, std::size_t hi) {
        if (lo < hi) {
            auto const p = partition(elems, lo, hi);
            quicksort(elems, lo, p);
            quicksort(elems, p + 1, hi);
        }
    }

    template<typename T>
    CIB_CONSTEVAL static void quicksort(T & collection) {
        quicksort(std::begin(collection), 0, std::size(collection) - std::size_t{1});
    }
}


#endif //COMPILE_TIME_INIT_BUILD_QUICKSORT_HPP
