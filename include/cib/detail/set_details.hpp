#include <cib/tuple.hpp>

#include <string_view>
#include <array>


#ifndef CIB_SET_DETAILS_HPP
#define CIB_SET_DETAILS_HPP


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

namespace cib::detail {
    struct typename_map_entry {
        std::string_view name;
        std::size_t index;
        std::size_t src;

        constexpr auto operator<(typename_map_entry rhs) const {
            return name < rhs.name;
        }

        constexpr auto operator<=(typename_map_entry rhs) const {
            return name <= rhs.name;
        }

        constexpr auto operator>(typename_map_entry rhs) const {
            return name > rhs.name;
        }

        constexpr auto operator>=(typename_map_entry rhs) const {
            return name >= rhs.name;
        }

        constexpr auto operator==(typename_map_entry rhs) const {
            return name == rhs.name;
        }

        constexpr auto operator!=(typename_map_entry rhs) const {
            return name != rhs.name;
        }
    };

    template<typename Tag>
    CIB_CONSTEVAL static std::string_view name() {
        #if defined(__clang__)
            constexpr std::string_view function_name = __PRETTY_FUNCTION__;
            constexpr auto lhs = 44;
            constexpr auto rhs = function_name.size() - 2;

        #elif defined(__GNUC__) || defined(__GNUG__)
            constexpr std::string_view function_name = __PRETTY_FUNCTION__;
            constexpr auto lhs = 59;
            constexpr auto rhs = function_name.size() - 51;

        #elif defined(_MSC_VER)
            constexpr std::string_view function_name = __FUNCSIG__;
            constexpr auto lhs = 0;
            constexpr auto rhs = function_name.size();
        #else
            static_assert(false, "Unknown compiler, can't build cib::tuple name.");
        #endif

        return function_name.substr(lhs, rhs - lhs + 1);
    }

    template<typename MetaFunc, typename... Types>
    CIB_CONSTEVAL static auto create_type_names(std::size_t src) {
        if constexpr (sizeof...(Types) == 0) {
            return std::array<typename_map_entry, 0>{};

        } else {
            std::size_t i = 0;

            std::array<typename_map_entry, sizeof...(Types)> names = {
                typename_map_entry{name<typename MetaFunc::template invoke<Types>>(), i++, src}...
            };

            detail::quicksort(names);
            return names;
        }
    }

    template<typename DataT, typename SizeT>
    struct data_and_size {
        DataT data;
        SizeT size;

        constexpr data_and_size(DataT data_arg, SizeT size_arg)
            : data{data_arg}
            , size{size_arg}
        {}
    };



    template<typename Algorithm, typename MetaFunc, typename LhsTuple, typename RhsTuple>
    struct set_operation_impl_t;


    template<
        typename Algorithm,
        typename MetaFunc,
        typename... LhsElems,
        typename... RhsElems>
    struct set_operation_impl_t<Algorithm, MetaFunc, tuple_impl<LhsElems...>, tuple_impl<RhsElems...>> {
        constexpr static auto result = [](){
            constexpr auto lhs_tags = create_type_names<MetaFunc, typename LhsElems::value_type...>(0);
            constexpr auto rhs_tags = create_type_names<MetaFunc, typename RhsElems::value_type...>(1);

            constexpr auto max_size =
                lhs_tags.size() + rhs_tags.size();

            std::array<detail::typename_map_entry, max_size> out{};

            auto const out_end =
                Algorithm::invoke(
                    lhs_tags.begin(), lhs_tags.end(),
                    rhs_tags.begin(), rhs_tags.end(),
                    out.begin());

            auto const out_size =
                std::distance(out.begin(), out_end);

            return data_and_size{out, out_size};
        };
    };

    template<
        int... Indices,
        typename IndexList,
        typename LhsTuple,
        typename RhsTuple>
    [[nodiscard]] constexpr auto merge_tuple_impl(
        std::integer_sequence<int, Indices...>,
        IndexList,
        LhsTuple lhs,
        RhsTuple rhs
    ) {
        auto const tuples =
            cib::make_tuple(lhs, rhs);

        return cib::make_tuple(tuples.get(index_<IndexList::result().data[Indices].src>).get(index_<IndexList::result().data[Indices].index>)...);
    }

    template<
        typename Algorithm,
        typename MetaFunc,
        typename LhsTuple,
        typename RhsTuple>
    [[nodiscard]] constexpr auto set_operation_impl(
        Algorithm,
        MetaFunc,
        LhsTuple lhs,
        RhsTuple rhs
    ) {
        using index_list_t =
            set_operation_impl_t<Algorithm, MetaFunc, LhsTuple, RhsTuple>;

        constexpr auto result_size =
            index_list_t::result().size;

        return merge_tuple_impl(
            std::make_integer_sequence<int, result_size>{},
            index_list_t{},
            lhs,
            rhs
        );
    }

    template<
        bool keep_left_difference,
        bool keep_intersection,
        bool keep_right_difference>
    struct set_operation_algorithm {
        template<typename InputIt1, typename InputIt2, typename OutputIt>
        constexpr static OutputIt invoke(
            InputIt1 first1, InputIt1 last1,
            InputIt2 first2, InputIt2 last2,
            OutputIt d_first
        ) {
            while (
                first1 != last1 &&
                first2 != last2
            ) {
                if (*first1 < *first2) {
                    if constexpr (keep_left_difference) {
                        *d_first++ = *first1;
                    }

                    first1++;

                } else if (*first1 > *first2) {
                    if constexpr (keep_right_difference) {
                        *d_first++ = *first2;
                    }

                    first2++;

                } else {
                    if constexpr (keep_intersection) {
                        *d_first++ = *first1;
                    }

                    first1++;
                    first2++;
                }
            }

            if constexpr (keep_left_difference) {
                while (first1 != last1) {
                    *d_first++ = *first1++;
                }
            }

            if constexpr (keep_right_difference) {
                while (first2 != last2) {
                    *d_first++ = *first2++;
                }
            }

            return d_first;
        }
    };
}


#endif //CIB_SET_DETAILS_HPP
