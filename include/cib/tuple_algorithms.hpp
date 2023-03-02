#pragma once

#include <cib/tuple.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <utility>

namespace cib {

// Algorithms for tuples:
//
// - tuple_cat: same as std::tuple_cat, but faster compilation
// - filter: compile-time filter on types
// - apply: same as std::apply (note: apply is also a member function on tuples,
//   which is slightly faster to compile)
// - transform, for_each, all_of, any_of, none_of: these all take the operation
//   as the first parameter in order to be variadic in the tuples and support
//   n-ary functions (this is a difference from boost::hana)
// - contains_type: a compile-time function (variable template) to test whether
//   a tuple contains a given type
//
// Note: where algorithms return a new tuple, it is not an indexed_tuple
// (although this could be added for appropriate use cases)

template <typename... Ts> [[nodiscard]] constexpr auto tuple_cat(Ts &&...ts) {
    if constexpr (sizeof...(Ts) == 0) {
        return cib::tuple<>{};
    } else if constexpr (sizeof...(Ts) == 1) {
        return (ts, ...);
    } else {
        constexpr auto total_num_elements =
            (std::size_t{} + ... + cib::tuple_size_v<std::remove_cvref_t<Ts>>);

        [[maybe_unused]] constexpr auto element_indices = [&] {
            std::array<detail::index_pair, total_num_elements> indices{};
            auto p = std::data(indices);
            ((p = std::remove_cvref_t<Ts>::fill_inner_indices(p)), ...);
            auto q = std::data(indices);
            std::size_t n{};
            ((q = std::remove_cvref_t<Ts>::fill_outer_indices(q, n++)), ...);
            return indices;
        }();

        [[maybe_unused]] auto outer_tuple =
            cib::tuple<Ts &&...>{std::forward<Ts>(ts)...};
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            using T = cib::tuple<cib::tuple_element_t<
                element_indices[Is].inner,
                std::remove_cvref_t<
                    decltype(std::move(outer_tuple)
                                 .ugly_iGet_rvr(
                                     index<element_indices[Is].outer>))>>...>;
            return T{std::move(outer_tuple)
                         .ugly_iGet_rvr(index<element_indices[Is].outer>)
                             [index<element_indices[Is].inner>]...};
        }
        (std::make_index_sequence<total_num_elements>{});
    }
}

template <template <typename T> typename Pred, typename T>
[[nodiscard]] constexpr auto filter(T &&t) {
    using tuple_t = std::remove_cvref_t<T>;
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        constexpr auto num_matches =
            (std::size_t{} + ... +
             (Pred<cib::tuple_element_t<Is, tuple_t>>::value ? std::size_t{1}
                                                             : std::size_t{}));
        constexpr auto indices = [] {
            auto a = std::array<std::size_t, num_matches>{};
            [[maybe_unused]] auto it = std::begin(a);
            [[maybe_unused]] auto copy_index =
                [&]<std::size_t I, typename Elem> {
                    if constexpr (Pred<Elem>::value) {
                        *it++ = I;
                    }
                };
            (copy_index
                 .template operator()<Is, cib::tuple_element_t<Is, tuple_t>>(),
             ...);
            return a;
        }();

        return [&]<std::size_t... Js>(std::index_sequence<Js...>) {
            using R = cib::tuple<cib::tuple_element_t<indices[Js], tuple_t>...>;
            return R{std::forward<T>(t)[index<indices[Js]>]...};
        }
        (std::make_index_sequence<num_matches>{});
    }
    (std::make_index_sequence<cib::tuple_size_v<tuple_t>>{});
}

namespace detail {
template <std::size_t I, typename... Ts>
constexpr auto invoke_at(auto &&op, Ts &&...ts) -> decltype(auto) {
    return op(std::forward<Ts>(ts)[index<I>]...);
}
} // namespace detail

template <template <typename> typename... Fs, typename Op, typename T,
          typename... Ts>
constexpr auto transform(Op &&op, T &&t, Ts &&...ts) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        if constexpr (sizeof...(Fs) == 0) {
            return cib::make_tuple(
                detail::invoke_at<Is>(std::forward<Op>(op), std::forward<T>(t),
                                      std::forward<Ts>(ts)...)...);
        } else {
            return cib::make_indexed_tuple<Fs...>(
                detail::invoke_at<Is>(std::forward<Op>(op), std::forward<T>(t),
                                      std::forward<Ts>(ts)...)...);
        }
    }
    (std::make_index_sequence<cib::tuple_size_v<std::remove_cvref_t<T>>>{});
}

template <typename Op, typename T>
constexpr auto apply(Op &&op, T &&t) -> decltype(auto) {
    return std::forward<T>(t).apply(std::forward<Op>(op));
}

template <typename Op, typename T, typename... Ts>
constexpr auto for_each(Op &&op, T &&t, Ts &&...ts) -> Op {
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (detail::invoke_at<Is>(op, std::forward<T>(t), std::forward<Ts>(ts)...),
         ...);
    }
    (std::make_index_sequence<cib::tuple_size_v<std::remove_cvref_t<T>>>{});
    return op;
}

template <typename F, typename T, typename... Ts>
constexpr auto all_of(F &&f, T &&t, Ts &&...ts) -> bool {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return (... and detail::invoke_at<Is>(f, std::forward<T>(t),
                                              std::forward<Ts>(ts)...));
    }
    (std::make_index_sequence<cib::tuple_size_v<std::remove_cvref_t<T>>>{});
}

template <typename F, typename T, typename... Ts>
constexpr auto any_of(F &&f, T &&t, Ts &&...ts) -> bool {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return (... or detail::invoke_at<Is>(f, std::forward<T>(t),
                                             std::forward<Ts>(ts)...));
    }
    (std::make_index_sequence<cib::tuple_size_v<std::remove_cvref_t<T>>>{});
}

template <typename... Ts> constexpr auto none_of(Ts &&...ts) -> bool {
    return not any_of(std::forward<Ts>(ts)...);
}

template <typename Tuple, typename T>
constexpr auto contains_type =
    []<std::size_t... Is>(std::index_sequence<Is...>) {
    return (... or std::is_same_v<T, cib::tuple_element_t<Is, Tuple>>);
}
(std::make_index_sequence<cib::tuple_size_v<Tuple>>{});

} // namespace cib
