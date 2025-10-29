#pragma once

#include <match/ops.hpp>
#include <match/sum_of_products.hpp>
#include <msg/field.hpp>
#include <msg/field_matchers.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/env.hpp>
#include <stdx/iterator.hpp>
#include <stdx/ranges.hpp>
#include <stdx/span.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/function.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/set.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <type_traits>

namespace msg {
template <auto V> struct constant_t {};
template <auto V> constexpr static auto constant = constant_t<V>{};

template <typename T>
concept matcher_maker = requires { typename T::is_matcher_maker; };

namespace detail {
template <stdx::ct_string N> struct matching_name {
    template <typename Field>
    using fn = std::bool_constant<N == Field::name_t::value>;
};

template <stdx::ct_string Name, typename RelOp, auto V> struct matcher_maker {
    using is_matcher_maker = void;

    template <typename Msg>
    constexpr static auto make_matcher() -> match::matcher auto {
        using fields_t = typename Msg::fields_t;
        using index_t =
            boost::mp11::mp_find_if_q<fields_t, matching_name<Name>>;
        static_assert(index_t::value < boost::mp11::mp_size<fields_t>::value,
                      "Named field not in message!");
        return rel_matcher_t<RelOp, boost::mp11::mp_at<fields_t, index_t>, V>{};
    }
};

template <msg::matcher_maker T> struct mm_not_t {
    using is_matcher_maker = void;
    template <typename Msg>
    constexpr static auto make_matcher() -> match::matcher auto {
        return not T::template make_matcher<Msg>();
    }
};

template <msg::matcher_maker T> constexpr auto operator not(T) -> mm_not_t<T> {
    return {};
}

template <msg::matcher_maker... Ts> struct mm_and_t {
    using is_matcher_maker = void;
    template <typename Msg>
    constexpr static auto make_matcher() -> match::matcher auto {
        return (... and Ts::template make_matcher<Msg>());
    }
};

template <msg::matcher_maker T, msg::matcher_maker U>
constexpr auto operator and(T, U) -> mm_and_t<T, U> {
    return {};
}

template <msg::matcher_maker... Ts> struct mm_or_t {
    using is_matcher_maker = void;
    template <typename Msg>
    constexpr static auto make_matcher() -> match::matcher auto {
        return (... or Ts::template make_matcher<Msg>());
    }
};

template <msg::matcher_maker T, msg::matcher_maker U>
constexpr auto operator or(T, U) -> mm_or_t<T, U> {
    return {};
}

template <match::matcher M> struct matcher_wrapper {
    using is_matcher_maker = void;

    template <typename>
    constexpr static auto make_matcher() -> match::matcher auto {
        return M{};
    }
};

template <match::matcher T, msg::matcher_maker U>
constexpr auto operator and(T, U) -> mm_and_t<matcher_wrapper<T>, U> {
    return {};
}
template <msg::matcher_maker T, match::matcher U>
constexpr auto operator and(T, U) -> mm_and_t<T, matcher_wrapper<U>> {
    return {};
}
template <match::matcher T, msg::matcher_maker U>
constexpr auto operator or(T, U) -> mm_or_t<matcher_wrapper<T>, U> {
    return {};
}
template <msg::matcher_maker T, match::matcher U>
constexpr auto operator or(T, U) -> mm_or_t<T, matcher_wrapper<U>> {
    return {};
}

template <stdx::ct_string Name, typename T> struct field_value {
    using name_t = stdx::cts_t<Name>;
    T value;
};

template <stdx::ct_string Name> struct field_name {
    using name_t = stdx::cts_t<Name>;

    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    template <typename T> constexpr auto operator=(T value) const {
        return field_value<Name, T>{value};
    }

    template <auto... Vs>
    constexpr static inline auto in =
        mm_or_t<matcher_maker<Name, std::equal_to<>, Vs>...>{};

  private:
    template <auto V>
    friend CONSTEVAL auto operator==(field_name, constant_t<V>)
        -> matcher_maker<Name, std::equal_to<>, V> {
        return {};
    }
    template <auto V>
    friend CONSTEVAL auto operator!=(field_name, constant_t<V>)
        -> matcher_maker<Name, std::not_equal_to<>, V> {
        return {};
    }
    template <auto V>
    friend CONSTEVAL auto operator<(field_name, constant_t<V>)
        -> matcher_maker<Name, std::less<>, V> {
        return {};
    }
    template <auto V>
    friend CONSTEVAL auto operator<=(field_name, constant_t<V>)
        -> matcher_maker<Name, std::less_equal<>, V> {
        return {};
    }
    template <auto V>
    friend CONSTEVAL auto operator>(field_name, constant_t<V>)
        -> matcher_maker<Name, std::greater<>, V> {
        return {};
    }
    template <auto V>
    friend CONSTEVAL auto operator>=(field_name, constant_t<V>)
        -> matcher_maker<Name, std::greater_equal<>, V> {
        return {};
    }
};
} // namespace detail

inline namespace literals {
template <stdx::ct_string S> constexpr auto operator""_field() {
    return detail::field_name<S>{};
}
template <stdx::ct_string S> constexpr auto operator""_f() {
    return detail::field_name<S>{};
}
} // namespace literals

namespace detail {
template <typename T>
concept some_field_value = requires(T const &t) {
    { t.value };
    typename T::name_t;
};

template <typename... Fields> struct storage_size {
    template <typename T>
    constexpr static std::size_t in =
        std::max({std::size_t{}, Fields::template extent_in<T>()...});
};

template <typename F> using name_for = typename F::name_t;

template <stdx::ct_string Name, typename... Fields> class msg_access {
    using FieldsTuple =
        decltype(stdx::make_indexed_tuple<name_for>(Fields{}...));

    template <typename N, stdx::range R> constexpr static auto check() {
        static_assert((std::is_same_v<N, name_for<Fields>> or ...),
                      "Field does not belong to this message!");
        using Field = field_t<N>;
        constexpr auto belongs = (std::is_same_v<typename Field::field_id,
                                                 typename Fields::field_id> or
                                  ...);
        static_assert(belongs, "Field does not belong to this message!");
        static_assert(Field::template fits_inside<std::remove_cvref_t<R>>(),
                      "Field does not fit inside message!");
    }

    template <stdx::range R, some_field_value V>
    constexpr static auto set1(R &&r, V v) -> void {
        check<name_for<V>, std::remove_cvref_t<R>>();
        using Field = field_t<name_for<V>>;
        Field::insert(std::forward<R>(r),
                      static_cast<typename Field::value_type>(v.value));
    }

    template <typename N, stdx::range R>
    constexpr static auto set_default(R &&r) -> void {
        check<N, std::remove_cvref_t<R>>();
        field_t<N>::insert_default(std::forward<R>(r));
    }

    template <typename N, stdx::range R> constexpr static auto get(R &&r) {
        check<N, std::remove_cvref_t<R>>();
        return field_t<N>::extract(std::forward<R>(r));
    }

  public:
    template <typename N>
    using field_t = std::remove_cvref_t<decltype(stdx::get<N>(FieldsTuple{}))>;

    template <stdx::range R, stdx::ct_string... Ns>
    constexpr static auto set(R &&r, field_name<Ns>...) -> void {
        (set_default<Ns>(r), ...);
    }

    template <stdx::range R, some_field_value... Vs>
    constexpr static auto set(R &&r, Vs... vs) -> void {
        (set1(r, vs), ...);
    }

    template <stdx::range R, typename... Fs>
    constexpr static auto set(R &&r, Fs...) -> void {
        (set_default<name_for<Fs>>(r), ...);
    }

    template <stdx::range R, stdx::ct_string N>
    constexpr static auto get(R &&r, field_name<N>) {
        return get<stdx::cts_t<N>>(std::forward<R>(r));
    }

    template <stdx::range R, typename F> constexpr static auto get(R &&r, F) {
        return get<name_for<F>>(std::forward<R>(r));
    }

    template <stdx::range R>
    [[nodiscard]] constexpr static auto describe(R &&r) {
        using namespace stdx::literals;
        auto const descs = [&] {
            auto const field_descriptions =
                stdx::tuple{Fields::describe(Fields::extract(r))...};
            if constexpr (sizeof...(Fields) > 0) {
                return field_descriptions.join(
                    [](auto lhs, auto rhs) { return lhs + ", "_ctst + rhs; });
            } else {
                return ""_ctst;
            }
        }();
        return stdx::ct_format<"{}({})">(stdx::cts_t<Name>{}, descs);
    }

    using default_value_type = std::uint32_t;

    template <template <typename, std::size_t> typename C, typename T>
    using storage_t = C<T, storage_size<Fields...>::template in<T>>;
    using default_storage_t = storage_t<std::array, default_value_type>;

    template <typename T>
    using span_t = stdx::span<T, storage_size<Fields...>::template in<T>>;
    using default_span_t = span_t<default_value_type>;
    using default_const_span_t = span_t<default_value_type const>;
};

template <typename T>
concept storage_like = stdx::range<T> and requires {
    { stdx::ct_capacity_v<T> } -> stdx::same_as_unqualified<std::size_t>;
    typename T::value_type;
};

template <stdx::ct_string Name, typename Env, typename... Fields>
struct message;

template <stdx::ct_string Name, typename Env> struct msg_q {
    template <typename... Fields> using fn = message<Name, Env, Fields...>;
};

template <typename F1, typename F2>
using field_sort_fn = std::bool_constant < F1::sort_key<F2::sort_key>;

template <typename F1, typename F2>
using name_equal_fn = std::is_same<name_for<F1>, name_for<F2>>;

template <typename... Fields>
using unique_by_name = boost::mp11::mp_unique_if<
    boost::mp11::mp_sort<boost::mp11::mp_list<Fields...>, field_sort_fn>,
    name_equal_fn>;

template <stdx::ct_string Name, typename Env, typename... Fields>
using message_without_unique_field_names =
    boost::mp11::mp_apply_q<detail::msg_q<Name, Env>,
                            detail::unique_by_name<Fields...>>;

template <stdx::ct_string Name, typename... Fields>
struct message_with_unique_field_names {
    static_assert(boost::mp11::mp_is_set<boost::mp11::mp_transform<
                      name_for, boost::mp11::mp_list<Fields...>>>::value,
                  "Message contains fields with duplicate names");

    using type =
        message_without_unique_field_names<Name, stdx::env<>, Fields...>;
};

template <stdx::ct_string Name, stdx::envlike Env, typename... Fields>
struct message_with_unique_field_names<Name, Env, Fields...> {
    static_assert(boost::mp11::mp_is_set<boost::mp11::mp_transform<
                      name_for, boost::mp11::mp_list<Fields...>>>::value,
                  "Message contains fields with duplicate names");

    using type = message_without_unique_field_names<Name, Env, Fields...>;
};

template <stdx::ct_string Name, typename Access, typename T> struct msg_base {
    constexpr static auto name = Name;

    constexpr auto as_derived() const -> T const & {
        return static_cast<T const &>(*this);
    }
    constexpr auto as_derived() -> T & { return static_cast<T &>(*this); }

    [[nodiscard]] constexpr auto get(auto f) const {
        return Access::get(as_derived().data(), f);
    }

    constexpr auto set(auto... fs) -> void {
        Access::set(as_derived().data(), fs...);
    }
    constexpr auto set() -> void {}

    template <stdx::ct_string N> struct proxy {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
        msg_base &b;

        // NOLINTNEXTLINE(misc-unconventional-assign-operator)
        constexpr auto operator=(auto val) const && -> void {
            b.set(field_name<N>{} = val);
        }

        using V = decltype(b.get(std::declval<field_name<N>>()));

        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr operator V() const { return b.get(field_name<N>{}); }
    };

    template <stdx::ct_string N>
    [[nodiscard]] constexpr auto operator[](field_name<N> f) const {
        return get(f);
    }
    template <stdx::ct_string N>
    [[nodiscard]] constexpr auto operator[](field_name<N>) LIFETIMEBOUND {
        return proxy<N>{*this};
    }

    [[nodiscard]] constexpr auto describe() const {
        return Access::describe(as_derived().data());
    }
};

template <stdx::ct_string Name, typename Env, typename... Fields>
struct message {
    using fields_t = stdx::type_list<Fields...>;
    using num_fields_t = std::integral_constant<std::size_t, sizeof...(Fields)>;
    template <std::size_t I> using nth_field_t = stdx::nth_t<I, Fields...>;

    using name_t = stdx::cts_t<Name>;
    using env_t = Env;
    using access_t = msg_access<Name, Fields...>;
    using default_storage_t = typename access_t::default_storage_t;
    using default_span_t = typename access_t::default_span_t;
    using default_const_span_t = typename access_t::default_const_span_t;
    template <std::unsigned_integral T>
    using size =
        std::integral_constant<std::size_t,
                               detail::storage_size<Fields...>::template in<T>>;

    template <stdx::ct_string N>
    using field_t = typename access_t::template field_t<stdx::cts_t<N>>;

    template <template <typename, std::size_t> typename C, typename T>
    using custom_storage_t = typename access_t::template storage_t<C, T>;

    template <auto N, typename Unit = bit_unit>
    using shifted_by =
        message<Name, Env, typename Fields::template shifted_by<N, Unit>...>;

    template <typename S>
    constexpr static auto fits_inside =
        (... and Fields::template fits_inside<S>());

    template <typename T> using base = msg_base<Name, access_t, T>;

    template <typename> struct owner_t;

    template <typename Span> struct view_t : base<view_t<Span>> {
        using definition_t = message;
        using span_t = Span;

        template <detail::storage_like S>
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) view_t(S const &s) : storage{s} {}

        template <detail::storage_like S, some_field_value... Vs>
        constexpr explicit view_t(S &s, Vs... vs) : storage{s} {
            this->set(vs...);
        }

        template <typename S>
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) view_t(owner_t<S> const &s LIFETIMEBOUND)
            : storage{s.data()} {}

        template <typename S, some_field_value... Vs>
        constexpr explicit(true) view_t(owner_t<S> &s LIFETIMEBOUND, Vs... vs)
            : storage{s.data()} {
            this->set(vs...);
        }

        template <typename S>
            requires(not std::same_as<S, span_t> and
                     std::same_as<std::add_const_t<typename S::element_type>,
                                  typename span_t::element_type> and
                     span_t::extent <= S::extent)
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) view_t(view_t<S> const &s)
            : storage{s.data()} {}

        [[nodiscard]] constexpr auto data() const { return storage; }

        [[nodiscard]] constexpr auto as_owning() { return owner_t{*this}; }

        using const_view_t =
            view_t<stdx::span<std::add_const_t<typename Span::value_type>,
                              stdx::ct_capacity_v<Span>>>;
        [[nodiscard]] constexpr auto as_const_view() const {
            return const_view_t{*this};
        }

      private:
        static_assert(definition_t::fits_inside<span_t>,
                      "Fields overflow message storage!");
        span_t storage{};

        friend constexpr auto operator==(view_t, view_t) -> bool {
            static_assert(stdx::always_false_v<Span>,
                          "Equality is not defined for messages: "
                          "consider using equivalent() instead.");
            return false;
        }

        using const_span_t =
            stdx::span<std::add_const_t<typename Span::value_type>,
                       stdx::ct_capacity_v<Span>>;
        using mutable_span_t =
            stdx::span<typename Span::value_type, stdx::ct_capacity_v<Span>>;

        friend constexpr auto equiv(view_t lhs, view_t<const_span_t> rhs)
            -> bool {
            return (... and (lhs.get(Fields{}) == rhs.get(Fields{})));
        }

        friend constexpr auto equiv(view_t lhs, view_t<mutable_span_t> rhs)
            -> bool {
            return equiv(lhs, rhs.as_const_view());
        }
    };
    using const_view_t = view_t<default_const_span_t>;
    using mutable_view_t = view_t<default_span_t>;

    template <typename Storage = default_storage_t>
    struct owner_t : base<owner_t<Storage>> {
        using definition_t = message;
        using storage_t = Storage;

        constexpr owner_t() {
            using uninit_fields =
                boost::mp11::mp_remove_if<boost::mp11::mp_list<Fields...>,
                                          has_default_value_t>;
            static_assert(boost::mp11::mp_empty<uninit_fields>::value,
                          "All fields must be initialized or defaulted");
            this->set(Fields{}...);
        }

        template <some_field_value... Vs> constexpr explicit owner_t(Vs... vs) {
            using defaulted_fields = boost::mp11::mp_transform<
                name_for,
                boost::mp11::mp_copy_if<boost::mp11::mp_list<Fields...>,
                                        has_default_value_t>>;
            using initialized_fields =
                boost::mp11::mp_transform<name_for,
                                          boost::mp11::mp_list<Vs...>>;

            using all_fields =
                boost::mp11::mp_transform<name_for,
                                          boost::mp11::mp_list<Fields...>>;
            using uninit_fields =
                boost::mp11::mp_set_difference<all_fields, defaulted_fields,
                                               initialized_fields>;
            static_assert(boost::mp11::mp_empty<uninit_fields>::value,
                          "All fields must be initialized or defaulted");

            this->set(Fields{}...);
            this->set(vs...);
        }

        template <detail::storage_like S, some_field_value... Vs>
        constexpr explicit owner_t(S const &s, Vs... vs) {
            static_assert(std::is_same_v<typename S::value_type,
                                         typename storage_t::value_type>,
                          "Attempted to construct owning message with "
                          "incompatible storage");

            constexpr auto N = std::min(stdx::ct_capacity_v<S>,
                                        stdx::ct_capacity_v<storage_t>);
            std::copy_n(std::begin(s), N, std::begin(storage));
            this->set(vs...);
        }

        template <detail::storage_like S, some_field_value... Vs>
        constexpr explicit owner_t(view_t<S> s, Vs... vs)
            : owner_t{s.data(), vs...} {}

        [[nodiscard]] constexpr auto data() LIFETIMEBOUND {
            return stdx::span{storage};
        }
        [[nodiscard]] constexpr auto data() const LIFETIMEBOUND {
            return stdx::span{storage};
        }

        [[nodiscard]] constexpr auto as_mutable_view() LIFETIMEBOUND {
            return view_t{*this};
        }

        using const_view_t =
            view_t<stdx::span<std::add_const_t<typename Storage::value_type>,
                              stdx::ct_capacity_v<Storage>>>;
        [[nodiscard]] constexpr auto as_const_view() const LIFETIMEBOUND {
            return view_t{*this};
        }

      private:
        static_assert(definition_t::fits_inside<storage_t>,
                      "Fields overflow message storage!");
        storage_t storage{};

        friend constexpr auto operator==(owner_t const &, owner_t const &)
            -> bool {
            static_assert(stdx::always_false_v<Storage>,
                          "Equality is not defined for messages: "
                          "consider using equivalent() instead.");
            return false;
        }

        using const_span_t = stdx::span<typename Storage::value_type const,
                                        stdx::ct_capacity_v<Storage>>;
        using mutable_span_t = stdx::span<typename Storage::value_type,
                                          stdx::ct_capacity_v<Storage>>;

        friend constexpr auto equiv(owner_t const &lhs,
                                    view_t<const_span_t> rhs) -> bool {
            return (... and (lhs.get(Fields{}) == rhs.get(Fields{})));
        }

        friend constexpr auto equiv(owner_t const &lhs,
                                    view_t<mutable_span_t> rhs) -> bool {
            return equiv(lhs, rhs.as_const_view());
        }
    };

    template <detail::storage_like S>
    owner_t(S const &, auto &&...)
        -> owner_t<std::array<typename S::value_type, stdx::ct_capacity_v<S>>>;
    template <some_field_value... Vs>
    owner_t(Vs...) -> owner_t<default_storage_t>;
    template <typename S>
    owner_t(view_t<S> &, auto &&...)
        -> owner_t<std::array<typename S::value_type, stdx::ct_capacity_v<S>>>;

    template <detail::storage_like S>
    view_t(S const &)
        -> view_t<stdx::span<std::add_const_t<typename S::value_type>,
                             stdx::ct_capacity_v<S>>>;
    template <detail::storage_like S>
    view_t(S &)
        -> view_t<stdx::span<typename S::value_type, stdx::ct_capacity_v<S>>>;

    template <typename T, std::size_t N>
        requires(std::is_const_v<T>)
    view_t(stdx::span<T, N>) -> view_t<stdx::span<T, N>>;
    template <typename T, std::size_t N>
        requires(not std::is_const_v<T>)
    view_t(stdx::span<T, N>, auto &&...) -> view_t<stdx::span<T, N>>;

    template <typename S>
    view_t(owner_t<S> const &) -> view_t<
        stdx::span<typename S::value_type const, stdx::ct_capacity_v<S>>>;
    template <typename S>
    view_t(owner_t<S> &, auto &&...)
        -> view_t<stdx::span<typename S::value_type, stdx::ct_capacity_v<S>>>;

    using matcher_t = decltype(match::all(typename Fields::matcher_t{}...));

    // NewFields go first because they override existing fields of the same
    // name (see unique_by_name)
    template <stdx::ct_string NewName, typename... NewFields>
    using extension =
        message_without_unique_field_names<NewName, Env, NewFields...,
                                           Fields...>;

    template <stdx::envlike E>
    using with_env = message<Name, stdx::append_env_t<Env, E>, Fields...>;
};
} // namespace detail

template <stdx::ct_string Name, typename... Ts>
using message =
    typename detail::message_with_unique_field_names<Name, Ts...>::type;

template <typename T> using owning = typename T::template owner_t<>;
template <typename T> using mutable_view = typename T::mutable_view_t;
template <typename T> using const_view = typename T::const_view_t;

template <typename M>
concept messagelike =
    requires { typename std::remove_cvref_t<M>::definition_t; };
template <typename M>
concept viewlike =
    messagelike<M> and requires { typename std::remove_cvref_t<M>::span_t; };
template <typename M>
concept owninglike =
    messagelike<M> and requires { typename std::remove_cvref_t<M>::storage_t; };

template <typename V, typename Def>
concept view_of = std::same_as<typename V::definition_t, Def> and
                  Def::template fits_inside<typename V::span_t>;
template <typename V, typename Def>
concept const_view_of =
    view_of<V, Def> and std::is_const_v<typename V::span_t::element_type>;

template <typename Def, stdx::ct_string Name, typename... Fields>
using extend = typename Def::template extension<Name, Fields...>;

template <owninglike O,
          view_of<typename std::remove_cvref_t<O>::definition_t> V>
constexpr auto equivalent(O &&o, V v) -> bool {
    return equiv(std::forward<O>(o), v);
}

template <owninglike O,
          view_of<typename std::remove_cvref_t<O>::definition_t> V>
constexpr auto equivalent(V v, O &&o) -> bool {
    return equiv(std::forward<O>(o), v);
}

template <viewlike V1, view_of<typename V1::definition_t> V2>
constexpr auto equivalent(V1 v1, V2 v2) -> bool {
    return equiv(v1, v2);
}

template <owninglike O1, owninglike O2>
constexpr auto equivalent(O1 const &lhs, O2 const &rhs) {
    return equiv(lhs, rhs.as_const_view());
}

template <typename Msg, typename F, typename S, typename... Args>
__attribute__((flatten, always_inline)) constexpr auto
call_with_message(F &&f, S &&s, Args &&...args) -> decltype(auto) {
    if constexpr (requires {
                      std::forward<F>(f)(std::forward<S>(s),
                                         std::forward<Args>(args)...);
                  }) {
        return std::forward<F>(f)(std::forward<S>(s),
                                  std::forward<Args>(args)...);
    } else if constexpr (requires {
                             std::forward<F>(f)(
                                 typename Msg::view_t{std::forward<S>(s)},
                                 std::forward<Args>(args)...);
                         }) {
        return std::forward<F>(f)(typename Msg::view_t{std::forward<S>(s)},
                                  std::forward<Args>(args)...);
    } else {
        return std::forward<F>(f)(typename Msg::owner_t{std::forward<S>(s)},
                                  std::forward<Args>(args)...);
    }
}

namespace detail {
template <typename AlignTo, typename... Msgs>
using msg_sizes = stdx::type_list<typename Msgs::template size<AlignTo>...>;

template <typename AlignTo, typename... Msgs>
using msg_offsets = boost::mp11::mp_push_front<
    boost::mp11::mp_partial_sum<msg_sizes<AlignTo, Msgs...>,
                                std::integral_constant<std::size_t, 0>,
                                boost::mp11::mp_plus>,
    std::integral_constant<std::size_t, 0>>;

template <typename AlignTo> struct shift_msg_q {
    template <typename Offset, typename Msg>
    using fn = typename Msg::template shifted_by<Offset::value, AlignTo>;
};

template <typename AlignTo, typename... Msgs>
using shifted_msgs =
    boost::mp11::mp_transform_q<shift_msg_q<AlignTo>,
                                msg_offsets<AlignTo, Msgs...>,
                                stdx::type_list<Msgs..., msg::message<"end">>>;

template <stdx::ct_string Name, stdx::envlike Env> struct overlayer {
    template <typename... Fields> using fn = msg::message<Name, Env, Fields...>;
};

template <stdx::ct_string Name> struct overlay_q {
    template <typename... Msgs>
        requires(sizeof...(Msgs) > 0)
    using fn = boost::mp11::mp_apply_q<
        overlayer<Name, stdx::append_env_t<typename Msgs::env_t...>>,
        boost::mp11::mp_append<typename Msgs::fields_t...>>;
};
} // namespace detail

template <stdx::ct_string Name, typename... Msgs>
    requires(sizeof...(Msgs) > 0)
using overlay = typename detail::overlay_q<Name>::template fn<Msgs...>;

template <stdx::ct_string Name, typename AlignTo, typename... Msgs>
    requires(sizeof...(Msgs) > 0)
using pack = boost::mp11::mp_apply_q<detail::overlay_q<Name>,
                                     detail::shifted_msgs<AlignTo, Msgs...>>;

namespace detail {
template <typename F>
using is_locatable = std::bool_constant<requires { F::sort_key; }>;

template <typename F1, typename F2>
using field_size_sort_fn = std::bool_constant < F2::bitsize<F1::bitsize>;

template <stdx::ct_string Name, typename... Fields>
struct field_locator : field_locator<Name, stdx::env<>, Fields...> {};

template <stdx::ct_string Name, stdx::envlike Env, typename... Fields>
struct field_locator<Name, Env, Fields...> {
    using fields = boost::mp11::mp_partition<boost::mp11::mp_list<Fields...>,
                                             is_locatable>;
    using located_fields = boost::mp11::mp_first<fields>;
    using unlocated_fields = boost::mp11::mp_second<fields>;

    using located_msg =
        boost::mp11::mp_apply_q<overlayer<Name, stdx::env<>>, located_fields>;

    using auto_fields =
        boost::mp11::mp_sort<unlocated_fields, field_size_sort_fn>;

    template <typename F>
    using as_singleton_message =
        msg::message<Name, Env, typename F::default_located>;
    using auto_msgs =
        boost::mp11::mp_transform<as_singleton_message, auto_fields>;

    using all_msgs = boost::mp11::mp_push_front<auto_msgs, located_msg>;

    template <typename... Msgs>
    using pack = msg::pack<Name, std::uint8_t, Msgs...>;
    using msg_type =
        typename boost::mp11::mp_apply<pack, all_msgs>::template with_env<Env>;
};
} // namespace detail

template <stdx::ct_string Name, typename... Ts>
using relaxed_message = typename detail::field_locator<Name, Ts...>::msg_type;

namespace detail {
template <typename Msg> struct replace_fields_q {
    template <typename... Fs>
    using fn = ::msg::message<Msg::name_t::value, typename Msg::env_t, Fs...>;
};

template <typename Msg, stdx::ct_string OldName, stdx::ct_string NewName>
struct with_renamed_field {
    using split_fields = boost::mp11::mp_partition_q<typename Msg::fields_t,
                                                     matching_name<OldName>>;

    using untouched_fs = boost::mp11::mp_second<split_fields>;
    static_assert(boost::mp11::mp_count_if_q<untouched_fs,
                                             matching_name<NewName>>::value ==
                      0,
                  "rename_field: New field name already exists in message");

    using replaced_fs = boost::mp11::mp_first<split_fields>;
    static_assert(not boost::mp11::mp_empty<replaced_fs>::value,
                  "rename_field: Old field name not found in message");

    using new_f = typename boost::mp11::mp_first<
        replaced_fs>::template with_new_name<NewName>;

    using new_fields = boost::mp11::mp_push_back<untouched_fs, new_f>;
    using msg = boost::mp11::mp_apply_q<replace_fields_q<Msg>, new_fields>;
};
} // namespace detail

template <typename Msg, stdx::ct_string OldName, stdx::ct_string NewName>
using rename_field =
    typename detail::with_renamed_field<Msg, OldName, NewName>::msg;
} // namespace msg
