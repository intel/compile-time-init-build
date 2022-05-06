#include "tuple.hpp"


#ifndef CIB_SET_HPP
#define CIB_SET_HPP


#include <string_view>
#include <array>

#include <algorithm>

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
        unsigned int index;
        unsigned int src;

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
    constexpr static std::string_view name() {
        #if defined(__clang__)
            constexpr std::string_view function_name = __PRETTY_FUNCTION__;
            constexpr auto lhs = 44;
            constexpr auto rhs = function_name.size() - 2;

        #elif defined(__GNUC__) || defined(__GNUG__)
            constexpr std::string_view function_name = __PRETTY_FUNCTION__;
            constexpr auto lhs = 59;
            constexpr auto rhs = function_name.size() - 51;

        #elif #elif defined(_MSC_VER)
            constexpr std::string_view function_name = __FUNCSIG__;
            constexpr auto lhs = 0;
            constexpr auto rhs = function_name.size();
        #else
            static_assert(false, "Unknown compiler, can't build cib::tuple name.");
        #endif

        return function_name.substr(lhs, rhs - lhs + 1);
    }

    template<typename MetaFunc, typename... Types>
    constexpr static auto create_type_names(unsigned int src) {
        if constexpr (sizeof...(Types) == 0) {
            return std::array<typename_map_entry, 0>{};

        } else {
            unsigned int i = 0;
            std::array<typename_map_entry, sizeof...(Types)> names = {
                typename_map_entry{name<typename MetaFunc::template invoke<Types>>(), i++, src}...
            };

            detail::quicksort(names);

            return names;
        }
    }

    template<typename MetaFunc, unsigned int src, typename... Types>
    constexpr static std::array<typename_map_entry, sizeof...(Types)> type_names =
        create_type_names<MetaFunc, Types...>(src);

}


namespace cib {
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
        constexpr static auto lhs_tags = detail::type_names<MetaFunc, 0, typename LhsElems::value_type...>;
        constexpr static auto rhs_tags = detail::type_names<MetaFunc, 1, typename RhsElems::value_type...>;

        constexpr static auto result = [](){
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
        }();
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
        constexpr auto index_list =
            IndexList::result.data;

        auto const tuples =
            cib::make_tuple(lhs, rhs);

        return make_tuple(tuples.get(index_<index_list[Indices].src>).get(index_<index_list[Indices].index>)...);
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

        constexpr auto result =
            index_list_t::result;

        return merge_tuple_impl(
            std::make_integer_sequence<int, result.size>{},
            index_list_t{},
            lhs,
            rhs
        );
    }

    struct set_union_algorithm {
        template<typename... ArgTs>
        constexpr static auto invoke(
            ArgTs... args
        ) {
            return std::set_union(args...);
        }
    };

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_union(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(set_union_algorithm{}, meta_func, lhs, rhs);
    }

    struct set_intersection_algorithm {
        template<typename... ArgTs>
        constexpr static auto invoke(
            ArgTs... args
        ) {
            return std::set_intersection(args...);
        }
    };

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_intersection(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(set_intersection_algorithm{}, meta_func, lhs, rhs);
    }

    struct set_difference_algorithm {
        template<typename... ArgTs>
        constexpr static auto invoke(
            ArgTs... args
        ) {
            return std::set_difference(args...);
        }
    };

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(set_difference_algorithm{}, meta_func, lhs, rhs);
    }

    struct set_symmetric_difference_algorithm {
        template<typename... ArgTs>
        constexpr static auto invoke(
            ArgTs... args
        ) {
            return std::set_symmetric_difference(args...);
        }
    };

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_symmetric_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(set_symmetric_difference_algorithm{}, meta_func, lhs, rhs);
    }
}

#endif //CIB_SET_HPP
