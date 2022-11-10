#pragma once

#include <cib/detail/compiler.hpp>

#include <array>
#if __has_include(<compare>)
#include <compare>
#endif
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cib {
template <typename T> struct tag_t {};
template <typename T> constexpr static tag_t<T> tag_{};

template <std::size_t Index> struct index_t {};
template <std::size_t Index> constexpr static index_t<Index> index_{};

template <typename T> struct index_metafunc_t {};
template <typename T> constexpr static index_metafunc_t<T> index_metafunc_{};

struct self_type {
    template <typename T> using invoke = T;
};
constexpr static index_metafunc_t<self_type> self_type_index{};

namespace detail {
template <typename ElementT, typename IndexT> struct type_indexed_element {
    [[nodiscard]] CIB_CONSTEXPR auto get(tag_t<IndexT>) const &noexcept
        -> decltype(auto) {
        return static_cast<ElementT const &>(*this).get(
            index_<ElementT::index>);
    }
    [[nodiscard]] CIB_CONSTEXPR auto get(tag_t<IndexT>) &noexcept
        -> decltype(auto) {
        return static_cast<ElementT &>(*this).get(index_<ElementT::index>);
    }
    [[nodiscard]] CIB_CONSTEXPR auto get(tag_t<IndexT>) &&noexcept
        -> decltype(auto) {
        return static_cast<ElementT &&>(*this).get(index_<ElementT::index>);
    }
};

template <typename T, std::size_t Index, typename... IndexTs>
struct tuple_element
    : type_indexed_element<tuple_element<T, Index, IndexTs...>, IndexTs>... {
    CIB_CONSTEXPR static auto index = Index;
    using value_type = T;
    value_type value{};

    CIB_CONSTEXPR tuple_element() = default;
    CIB_CONSTEXPR explicit tuple_element(T const &t) : value{t} {}
    CIB_CONSTEXPR explicit tuple_element(T &&t) : value{std::move(t)} {}

    [[nodiscard]] CIB_CONSTEXPR auto get(index_t<Index>) const &noexcept
        -> value_type const & {
        return value;
    }
    [[nodiscard]] CIB_CONSTEXPR auto get(index_t<Index>) &noexcept
        -> value_type & {
        return value;
    }
    [[nodiscard]] CIB_CONSTEXPR auto get(index_t<Index>) &&noexcept
        -> value_type && {
        return std::move(value);
    }

    using type_indexed_element<tuple_element, IndexTs>::get...;
};

/**
 * Used by fold_right to leverage c++17 fold expressions with arbitrary
 * callables.
 *
 * @tparam ValueType
 *      The type of the element from the value pack.
 *
 * @tparam CallableType
 *      A callable that takes two arguments, current element to be
 *      processed and the fold state.
 */
template <typename TValue, typename TOp> struct fold_right_helper {
    TValue value;
    TOp op;

  private:
    template <typename T>
    [[nodiscard]] CIB_CONSTEXPR friend inline auto
    operator+(T &&lhs, fold_right_helper &&rhs) {
        using R = decltype(rhs.op(std::forward<T>(lhs), std::move(rhs).value));
        return fold_right_helper<R, TOp>{
            rhs.op(std::forward<T>(lhs), std::move(rhs).value),
            std::move(rhs).op};
    }

    template <typename T>
    [[nodiscard]] CIB_CONSTEXPR friend inline auto
    operator+(fold_right_helper<T, TOp> &&lhs, fold_right_helper &&rhs) {
        using R =
            decltype(rhs.op(std::forward<fold_right_helper<T, TOp>>(lhs).value,
                            std::move(rhs).value));
        return fold_right_helper<R, TOp>{
            rhs.op(std::forward<fold_right_helper<T, TOp>>(lhs).value,
                   std::move(rhs).value),
            std::move(rhs).op};
    }
};

template <typename TValue, typename TOp>
fold_right_helper(TValue, TOp) -> fold_right_helper<std::decay_t<TValue>, TOp>;

template <typename TValue, typename TOp> struct fold_left_helper {
    TValue value;
    TOp op;

  private:
    template <typename T>
    [[nodiscard]] CIB_CONSTEXPR friend inline auto
    operator+(fold_left_helper &&lhs, T &&rhs) {
        using R = decltype(lhs.op(std::move(lhs).value, std::forward<T>(rhs)));
        return fold_left_helper<R, TOp>{
            lhs.op(std::move(lhs).value, std::forward<T>(rhs)),
            std::move(lhs).op};
    }

    template <typename T>
    [[nodiscard]] CIB_CONSTEXPR friend inline auto
    operator+(fold_left_helper &&lhs, fold_left_helper<T, TOp> &&rhs) {
        using R =
            decltype(lhs.op(std::move(lhs).value,
                            std::forward<fold_left_helper<T, TOp>>(rhs).value));
        return fold_left_helper<R, TOp>{
            lhs.op(std::move(lhs).value,
                   std::forward<fold_left_helper<T, TOp>>(rhs).value),
            std::move(lhs).op};
    }
};

template <typename TValue, typename TOp>
fold_left_helper(TValue, TOp) -> fold_left_helper<std::decay_t<TValue>, TOp>;

} // namespace detail

template <typename... TupleElementTs> struct tuple_impl : TupleElementTs... {
    using TupleElementTs::get...;

    [[nodiscard]] CIB_CONSTEXPR static auto size() -> std::size_t {
        return sizeof...(TupleElementTs);
    }

    template <typename Callable>
    CIB_CONSTEXPR auto apply(Callable &&operation) const & -> decltype(auto) {
        return std::forward<Callable>(operation)(TupleElementTs::value...);
    }

    template <typename Callable>
    CIB_CONSTEXPR auto apply(Callable &&operation) && -> decltype(auto) {
        return std::forward<Callable>(operation)(
            std::move(TupleElementTs::value)...);
    }

    /**
     * Perform an operation on each element.
     *
     * @param operation
     *      The operation to perform. Must be a callable that accepts a single
     * parameter.
     */
    template <typename TOp>
    CIB_CONSTEXPR auto for_each(TOp &&op) const & -> TOp {
        (op(TupleElementTs::value), ...);
        return op;
    }
    template <typename TOp> CIB_CONSTEXPR auto for_each(TOp &&op) & -> TOp {
        (op(TupleElementTs::value), ...);
        return op;
    }

    /**
     * fold_right a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template <typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto
    fold_right(TOp &&operation) const & {
        return (detail::fold_right_helper{TupleElementTs::value,
                                          std::forward<TOp>(operation)} +
                ...)
            .value;
    }
    template <typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_right(TOp &&operation) && {
        return (detail::fold_right_helper{std::move(TupleElementTs::value),
                                          std::forward<TOp>(operation)} +
                ...)
            .value;
    }

    /**
     * fold_right a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template <typename TInit, typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto
    fold_right(TInit &&initial_state, TOp &&operation) const & {
        return (TupleElementTs::value + ... +
                detail::fold_right_helper{std::forward<TInit>(initial_state),
                                          std::forward<TOp>(operation)})
            .value;
    }
    template <typename TInit, typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_right(TInit &&initial_state,
                                                       TOp &&operation) && {
        return (std::move(TupleElementTs::value) + ... +
                detail::fold_right_helper{std::forward<TInit>(initial_state),
                                          std::forward<TOp>(operation)})
            .value;
    }

    /**
     * fold_left a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template <typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_left(TOp &&operation) const & {
        return (... + detail::fold_left_helper{TupleElementTs::value,
                                               std::forward<TOp>(operation)})
            .value;
    }
    template <typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_left(TOp &&operation) && {
        return (... + detail::fold_left_helper{std::move(TupleElementTs::value),
                                               std::forward<TOp>(operation)})
            .value;
    }

    /**
     * fold_left a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template <typename TInit, typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_left(TInit &&initial_state,
                                                      TOp &&operation) const & {
        return (detail::fold_left_helper{std::forward<TInit>(initial_state),
                                         std::forward<TOp>(operation)} +
                ... + TupleElementTs::value)
            .value;
    }
    template <typename TInit, typename TOp>
    [[nodiscard]] CIB_CONSTEXPR inline auto fold_left(TInit &&initial_state,
                                                      TOp &&operation) && {
        return (detail::fold_left_helper{std::forward<TInit>(initial_state),
                                         std::forward<TOp>(operation)} +
                ... + std::move(TupleElementTs::value))
            .value;
    }

  private:
    [[nodiscard]] CIB_CONSTEXPR friend auto operator==(tuple_impl const &lhs,
                                                       tuple_impl const &rhs)
        -> bool {
        return ((lhs.TupleElementTs::value == rhs.TupleElementTs::value) and
                ...);
    }

#if __cpp_lib_three_way_comparison < 201907L
    [[nodiscard]] CIB_CONSTEXPR friend auto operator!=(tuple_impl const &lhs,
                                                       tuple_impl const &rhs)
        -> bool {
        return not(lhs == rhs);
    }

    [[nodiscard]] CIB_CONSTEXPR friend auto operator<(tuple_impl const &lhs,
                                                      tuple_impl const &rhs)
        -> bool {
        bool result{};
        const auto cmp = [&](auto const &x, auto const &y) {
            result = x < y;
            return result or x != y;
        };
        (void)(cmp(lhs.TupleElementTs::value, rhs.TupleElementTs::value) or
               ...);
        return result;
    }

    [[nodiscard]] CIB_CONSTEXPR friend auto operator>(tuple_impl const &lhs,
                                                      tuple_impl const &rhs)
        -> bool {
        return rhs < lhs;
    }

    [[nodiscard]] CIB_CONSTEXPR friend auto operator<=(tuple_impl const &lhs,
                                                       tuple_impl const &rhs)
        -> bool {
        return not(rhs < lhs);
    }

    [[nodiscard]] CIB_CONSTEXPR friend auto operator>=(tuple_impl const &lhs,
                                                       tuple_impl const &rhs)
        -> bool {
        return not(lhs < rhs);
    }
#else
    [[nodiscard]] CIB_CONSTEXPR friend auto
    operator<=>(tuple_impl const &lhs, tuple_impl const &rhs) requires(
        std::three_way_comparable<typename TupleElementTs::value_type> and...) {
        std::common_comparison_category_t<std::compare_three_way_result_t<
            typename TupleElementTs::value_type>...>
            result{std::strong_ordering::equivalent};
        const auto cmp = [&](auto const &x, auto const &y) {
            result = x <=> y;
            return result != 0;
        };
        (void)(cmp(lhs.TupleElementTs::value, rhs.TupleElementTs::value) or
               ...);
        return result;
    }
#endif
};

template <typename... Ts> tuple_impl(Ts...) -> tuple_impl<Ts...>;

namespace detail {
template <typename... TIndexMetafuncs> struct indexed_element_helper {
    template <typename T, std::size_t Index>
    using element_t =
        tuple_element<T, Index,
                      typename TIndexMetafuncs::template invoke<T>...>;
};

template <typename... TIndexMetafuncs, std::size_t... Indices, typename... Ts>
[[nodiscard]] constexpr auto make_tuple_impl(std::index_sequence<Indices...>,
                                             Ts &&...values) {
    using helper_t = indexed_element_helper<TIndexMetafuncs...>;
    return tuple_impl{
        typename helper_t::template element_t<std::decay_t<Ts>, Indices>{
            std::forward<Ts>(values)}...};
}

} // namespace detail

template <typename... Ts>
[[nodiscard]] constexpr auto make_tuple(Ts &&...values) {
    return detail::make_tuple_impl(
        std::make_index_sequence<sizeof...(values)>{},
        std::forward<Ts>(values)...);
}

template <typename IndexMetafuncT0, typename... Ts>
[[nodiscard]] constexpr auto make_tuple(index_metafunc_t<IndexMetafuncT0>,
                                        Ts &&...values) {
    return detail::make_tuple_impl<IndexMetafuncT0>(
        std::make_index_sequence<sizeof...(values)>{},
        std::forward<Ts>(values)...);
}

template <typename IndexMetafuncT0, typename IndexMetafuncT1, typename... Ts>
[[nodiscard]] constexpr auto make_tuple(index_metafunc_t<IndexMetafuncT0>,
                                        index_metafunc_t<IndexMetafuncT1>,
                                        Ts &&...values) {
    return detail::make_tuple_impl<IndexMetafuncT0, IndexMetafuncT1>(
        std::make_index_sequence<sizeof...(values)>{},
        std::forward<Ts>(values)...);
}

template <typename IndexMetafuncT0, typename IndexMetafuncT1,
          typename IndexMetafuncT2, typename... Ts>
[[nodiscard]] constexpr auto
make_tuple(index_metafunc_t<IndexMetafuncT0>, index_metafunc_t<IndexMetafuncT1>,
           index_metafunc_t<IndexMetafuncT2>, Ts &&...values) {
    return detail::make_tuple_impl<IndexMetafuncT0, IndexMetafuncT1,
                                   IndexMetafuncT2>(
        std::make_index_sequence<sizeof...(values)>{},
        std::forward<Ts>(values)...);
}

template <typename... Ts>
using tuple = decltype(cib::make_tuple(std::declval<Ts>()...));

template <typename Callable, typename Tuple>
CIB_CONSTEXPR auto apply(Callable &&operation, Tuple &&t) {
    return std::forward<Tuple>(t).apply(std::forward<Callable>(operation));
}

namespace detail {
template <std::size_t Index, typename TOp, typename... Tuples>
CIB_CONSTEXPR auto invoke_at(TOp &&op, Tuples &&...ts) -> void {
    std::forward<TOp>(op)(std::forward<Tuples>(ts).get(index_<Index>)...);
}

template <typename TOp, std::size_t... Indices, typename... Tuples>
CIB_CONSTEXPR auto for_each_impl(TOp &&op, std::index_sequence<Indices...>,
                                 Tuples &&...ts) -> TOp {
    (invoke_at<Indices>(op, std::forward<Tuples>(ts)...), ...);
    return op;
}
} // namespace detail

template <typename TOp, typename Tuple, typename... Tuples>
CIB_CONSTEXPR auto for_each(TOp &&op, Tuple &&t, Tuples &&...ts) -> TOp {
    return detail::for_each_impl(
        std::forward<TOp>(op),
        std::make_index_sequence<std::decay_t<Tuple>::size()>{},
        std::forward<Tuple>(t), std::forward<Tuples>(ts)...);
}

template <typename MetaFunc, typename Tuple, typename Operation>
[[nodiscard]] CIB_CONSTEXPR auto transform(MetaFunc meta_func, Tuple tuple,
                                           Operation op) {
    return tuple.apply([&](auto... elements) {
        return cib::make_tuple(meta_func, op(elements)...);
    });
}

template <typename Tuple, typename Operation>
[[nodiscard]] CIB_CONSTEXPR auto transform(Tuple tuple, Operation op) {
    return tuple.apply(
        [&](auto... elements) { return cib::make_tuple(op(elements)...); });
}

template <typename IntegralType, IntegralType... Indices, typename CallableType>
[[nodiscard]] CIB_CONSTEXPR auto
transform(std::integer_sequence<IntegralType, Indices...>,
          CallableType const &op) {
    return cib::make_tuple(
        op(std::integral_constant<IntegralType, Indices>{})...);
}

template <typename Tuple, typename Operation>
[[nodiscard]] CIB_CONSTEXPR auto filter(Tuple tuple, Operation op) {
    return tuple.apply([&](auto... elements) {
        std::array<bool, tuple.size()> constexpr op_results{
            op(decltype(elements){})...};

        // std::count
        auto constexpr num_matches = [&]() {
            int count = 0;

            for (bool v : op_results) {
                if (v) {
                    count += 1;
                }
            }

            return count;
        }();

        auto constexpr indices = [&]() {
            std::array<std::size_t, num_matches> result{};
            auto dst = std::size_t{};

            for (auto i = std::size_t{}; i < op_results.size(); i++) {
                if (op_results[i]) {
                    result[dst] = i;
                    dst += 1;
                }
            }

            return result;
        }();

        return transform(
            std::make_index_sequence<indices.size()>{},
            [&](auto i) { return tuple.get(index_<indices[i.value]>); });
    });
}

template <typename Operation>
[[nodiscard]] CIB_CONSTEXPR auto filter(cib::tuple_impl<> t, Operation) {
    return t;
}

namespace detail {
template <std::size_t... Indices, typename... Tuples>
constexpr auto tuple_cat_impl(std::index_sequence<Indices...>,
                              Tuples &&...tuples) {
    struct tuple_pair {
        std::size_t outer;
        std::size_t inner;
    };

    constexpr auto num_tuples = sizeof...(tuples);
    constexpr std::array<std::size_t, num_tuples> tuple_sizes{
        std::decay_t<Tuples>::size()...};
    [[maybe_unused]] constexpr auto element_indices = [&]() {
        constexpr auto total_num_elements =
            (std::decay_t<Tuples>::size() + ...);
        std::array<tuple_pair, total_num_elements> indices{};
        std::size_t flat_index = 0;
        for (std::size_t outer_index = 0; outer_index < num_tuples;
             outer_index++) {
            for (std::size_t inner_index = 0;
                 inner_index < tuple_sizes[outer_index]; inner_index++) {
                indices[flat_index] = {outer_index, inner_index};
                flat_index += 1;
            }
        }
        return indices;
    }();

    [[maybe_unused]] auto outer_tuple =
        cib::make_tuple(std::forward<Tuples>(tuples)...);

    return cib::make_tuple(std::move(outer_tuple)
                               .get(index_<element_indices[Indices].outer>)
                               .get(index_<element_indices[Indices].inner>)...);
}
} // namespace detail

template <typename... Tuples> constexpr auto tuple_cat(Tuples &&...tuples) {
    constexpr auto total_num_elements = (std::decay_t<Tuples>::size() + ...);
    return detail::tuple_cat_impl(
        std::make_index_sequence<total_num_elements>{},
        std::forward<Tuples>(tuples)...);
}
} // namespace cib

template <typename... Elements>
struct std::tuple_size<cib::tuple_impl<Elements...>>
    : std::integral_constant<std::size_t, sizeof...(Elements)> {};
