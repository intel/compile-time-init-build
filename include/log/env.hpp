#pragma once

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/algorithm.hpp>

#ifdef __clang__
#define CIB_PRAGMA_SEMI
#else
#define CIB_PRAGMA_SEMI ;
#endif

namespace logging {
template <auto Query, auto Value> struct prop {
    [[nodiscard]] CONSTEVAL static auto query(decltype(Query)) noexcept {
        return Value;
    }
};

namespace detail {
template <typename Q, typename Env>
concept valid_query_for = requires { Env::query(Q{}); };

template <typename Q, typename... Envs>
concept valid_query_over = (... or valid_query_for<Q, Envs>);

template <typename Q> struct has_query {
    template <typename Env>
    using fn = std::bool_constant<valid_query_for<Q, Env>>;
};
} // namespace detail

template <typename... Envs> struct env {
    template <detail::valid_query_over<Envs...> Q>
    CONSTEVAL static auto query(Q) noexcept {
        using I = boost::mp11::mp_find_if_q<boost::mp11::mp_list<Envs...>,
                                            detail::has_query<Q>>;
        using E = boost::mp11::mp_at<boost::mp11::mp_list<Envs...>, I>;
        return Q{}(E{});
    }
};

namespace detail {
template <typename T> struct autowrap {
    CONSTEVAL autowrap(T t) : value(t) {}
    T value;
};

template <std::size_t N> using str_lit_t = char const (&)[N];

template <std::size_t N> struct autowrap<str_lit_t<N>> {
    CONSTEVAL autowrap(str_lit_t<N> str) : value(str) {}
    stdx::ct_string<N> value;
};

template <typename T> autowrap(T) -> autowrap<T>;
template <std::size_t N> autowrap(str_lit_t<N>) -> autowrap<str_lit_t<N>>;

template <auto V> struct wrap {
    constexpr static auto value = V;
};

template <typename> struct for_each_pair;
template <std::size_t... Is> struct for_each_pair<std::index_sequence<Is...>> {
    template <auto... Args>
    using type = env<
        prop<boost::mp11::mp_at_c<boost::mp11::mp_list<wrap<Args>...>,
                                  2 * Is>::value.value,
             stdx::ct<boost::mp11::mp_at_c<boost::mp11::mp_list<wrap<Args>...>,
                                           2 * Is + 1>::value.value>()>...>;
};
} // namespace detail
} // namespace logging

using cib_log_env_t = logging::env<>;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CIB_LOG_ENV(...)                                                       \
    STDX_PRAGMA(diagnostic push)                                               \
    STDX_PRAGMA(diagnostic ignored "-Wshadow")                                 \
    using cib_log_env_t [[maybe_unused]] =                                     \
        decltype([]<logging::detail::autowrap... Args> {                       \
            using new_env_t = typename logging::detail::for_each_pair<         \
                std::make_index_sequence<sizeof...(Args) /                     \
                                         2>>::template type<Args...>;          \
            return boost::mp11::mp_append<new_env_t, cib_log_env_t>{};         \
        }.template operator()<__VA_ARGS__>()) CIB_PRAGMA_SEMI                  \
            STDX_PRAGMA(diagnostic pop)
// NOLINTEND(cppcoreguidelines-macro-usage)
