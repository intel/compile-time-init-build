#include "ordered_set.hpp"
#include "quicksort.hpp"

#include <string_view>
#include <array>


#ifndef COMPILE_TIME_INIT_BUILD_FIND_HPP
#define COMPILE_TIME_INIT_BUILD_FIND_HPP


namespace cib::detail {
    struct typename_map_entry {
        std::string_view name;
        unsigned int index;

        CIB_CONSTEVAL bool operator<(typename_map_entry rhs) const {
            return name < rhs.name;
        }

        CIB_CONSTEVAL bool operator>(typename_map_entry rhs) const {
            return name > rhs.name;
        }
    };

    // clang-8 and below don't have constexpr find_first_of on std::string_view
    CIB_CONSTEVAL std::string_view::size_type find_first_of(std::string_view str, char c) {
        using size_type = std::string_view::size_type;
        for (size_type i = 0; i < str.size(); i++) {
            if (str[i] == c) {
                return i;
            }
        }

        return 0;
    }

    // clang-8 and below don't have constexpr find_last_of on std::string_view
    CIB_CONSTEVAL std::string_view::size_type find_last_of(std::string_view str, char c) {
        using size_type = std::string_view::size_type;
        size_type match = str.size();
        for (size_type i = 0; i < str.size(); i++) {
            if (str[i] == c) {
                match = i;
            }
        }

        return match;
    }

    template<typename Tag>
    CIB_CONSTEVAL static std::string_view name() {
        constexpr std::string_view function_name = __PRETTY_FUNCTION__;
        constexpr auto lhs = find_first_of(function_name, '<');
        constexpr auto rhs = find_last_of(function_name, '>');
        constexpr auto service_name = function_name.substr(lhs, rhs - lhs);
        return service_name;
    }


    template<typename MetaFunc, typename... Types>
    CIB_CONSTEXPR static auto create_type_names() {
        unsigned int i = 0;
        std::array<typename_map_entry, sizeof...(Types)> names = {
            typename_map_entry{name<typename MetaFunc::template invoke<Types>>(), i++}...
        };

        quicksort(names);
        return names;
    }

    template<typename MetaFunc, typename... Types>
    constexpr static std::array<typename_map_entry, sizeof...(Types)> type_names =
        create_type_names<MetaFunc, Types...>();

    template<typename ElemType>
    CIB_CONSTEVAL static auto binary_search(
        ElemType const * elems,
        std::size_t num_elems,
        ElemType const & search_key
    ) {
        // https://en.wikipedia.org/wiki/Binary_search_algorithm#Procedure

        std::size_t left = 0;
        std::size_t right = num_elems - 1;

        while (left <= right) {
            auto const middle = (left + right) / 2;
            if (elems[middle] < search_key) {
                left = middle + 1;
            } else if (elems[middle] > search_key) {
                right = middle - 1;
            } else {
                return elems[middle];
            }
        }

        return ElemType{};
    }

    template<typename ContainerType, typename ElemType>
    CIB_CONSTEVAL static auto binary_search(
        ContainerType const & container,
        ElemType const & search_key
    ) {
        return binary_search(container.begin(), std::size(container), search_key);
    }

    template<typename Tag, typename MetaFunc, typename... Types>
    constexpr auto const & find(ordered_set<Types...> const & t) {

        #if defined(__GNUC__)
            // g++-9 doesn't compile when we use type_names<...> directly, so we have
            // to use the function again instead.
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64989
            constexpr auto index =
                binary_search(
                    create_type_names<MetaFunc, Types...>(),
                    typename_map_entry{name<Tag>(), 0}
                ).index;
        #else
            constexpr auto index =
                binary_search(
                    type_names<MetaFunc, Types...>,
                    typename_map_entry{name<Tag>(), 0}
                ).index;
        #endif

        return t.template get<index>();
    }



}


#endif //COMPILE_TIME_INIT_BUILD_FIND_HPP
