#pragma once

#include <match/ops.hpp>
#include <match/sum_of_products.hpp>
#include <msg/field.hpp>
#include <msg/field_matchers.hpp>
#include <sc/format.hpp>
#include <sc/fwd.hpp>

#include <stdx/compiler.hpp>
#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/iterator.hpp>
#include <stdx/span.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>
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
template <typename N> struct matching_name {
    template <typename Field>
    using fn = std::is_same<N, typename Field::name_t>;
};

template <typename Name, typename RelOp, auto V> struct matcher_maker {
    using is_matcher_maker = void;
    using name_t = Name;

    template <typename Msg>
    constexpr static auto make_matcher() -> match::matcher auto {
        using fields_t = typename Msg::fields_t;
        using index_t =
            boost::mp11::mp_find_if_q<fields_t, matching_name<name_t>>;
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

template <typename Name, typename T> struct field_value {
    using name_t = Name;
    T value;
};

template <typename Name> struct field_name {
    using name_t = Name;

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    template <typename T> constexpr auto operator=(T value) {
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

template <class T, T... chars> constexpr auto operator""_field() {
    return detail::field_name<sc::string_constant<T, chars...>>{};
}

template <typename T>
concept field_value = requires(T const &t) {
    { t.value };
    typename T::name_t;
};

namespace detail {
template <typename... Fields> struct storage_size {
    template <typename T>
    constexpr static std::size_t in =
        std::max({std::size_t{}, Fields::template extent_in<T>()...});
};

template <typename F> using name_for = typename F::name_t;

template <stdx::ct_string Name, typename... Fields> class message_access {
    using FieldsTuple =
        decltype(stdx::make_indexed_tuple<name_for>(Fields{}...));

    template <typename Field, stdx::range R> constexpr static auto check() {
        constexpr auto belongs = (std::is_same_v<typename Field::field_id,
                                                 typename Fields::field_id> or
                                  ...);
        static_assert(belongs, "Field does not belong to this message!");
        static_assert(Field::template fits_inside<std::remove_cvref_t<R>>(),
                      "Field does not fit inside message!");
    }

    template <stdx::range R, msg::field_value V>
    constexpr static auto set1(R &&r, V v) -> void {
        using Field =
            std::remove_cvref_t<decltype(stdx::get<typename V::name_t>(
                FieldsTuple{}))>;
        check<Field, std::remove_cvref_t<R>>();
        Field::insert(std::forward<R>(r),
                      static_cast<typename Field::value_type>(v.value));
    }

    template <typename N, stdx::range R>
    constexpr static auto set_default(R &&r) -> void {
        using Field =
            std::remove_cvref_t<decltype(stdx::get<N>(FieldsTuple{}))>;
        check<Field, std::remove_cvref_t<R>>();
        Field::insert(
            std::forward<R>(r),
            static_cast<typename Field::value_type>(Field::default_value));
    }

    template <typename N, stdx::range R> constexpr static auto get(R &&r) {
        using Field =
            std::remove_cvref_t<decltype(stdx::get<N>(FieldsTuple{}))>;
        check<Field, std::remove_cvref_t<R>>();
        return Field::extract(std::forward<R>(r));
    }

  public:
    template <stdx::range R, typename... Ns>
    constexpr static auto set(R &&r, field_name<Ns>...) -> void {
        (set_default<Ns>(r), ...);
    }

    template <stdx::range R, msg::field_value... Vs>
    constexpr static auto set(R &&r, Vs... vs) -> void {
        (set1(r, vs), ...);
    }

    template <stdx::range R, typename... Fs>
    constexpr static auto set(R &&r, Fs...) -> void {
        (set_default<typename Fs::name_t>(r), ...);
    }

    template <stdx::range R, typename N>
    constexpr static auto get(R &&r, field_name<N>) {
        return get<N>(std::forward<R>(r));
    }

    template <stdx::range R, typename F> constexpr static auto get(R &&r, F) {
        return get<typename F::name_t>(std::forward<R>(r));
    }

    template <stdx::range R>
    [[nodiscard]] constexpr static auto describe(R &&r) {
        using msg_name =
            decltype(stdx::ct_string_to_type<Name, sc::string_constant>());
        auto const descs = [&] {
            auto const field_descriptions =
                stdx::tuple{Fields::describe(Fields::extract(r))...};
            if constexpr (sizeof...(Fields) > 0) {
                return field_descriptions.join(
                    [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });
            } else {
                return ""_sc;
            }
        }();
        return format("{}({})"_sc, msg_name{}, descs);
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
} // namespace detail

template <stdx::ct_string Name, typename... Fields> struct message {
    using fields_t = stdx::type_list<Fields...>;
    using access_t = detail::message_access<Name, Fields...>;
    using default_storage_t = typename access_t::default_storage_t;
    using default_span_t = typename access_t::default_span_t;
    using default_const_span_t = typename access_t::default_const_span_t;

    template <template <typename, std::size_t> typename C, typename T>
    using custom_storage_t = typename access_t::template storage_t<C, T>;

    static_assert(
        boost::mp11::mp_is_set<boost::mp11::mp_transform<
            detail::name_for, boost::mp11::mp_list<Fields...>>>::value,
        "Message contains fields with duplicate names");

    template <typename T> struct base {
        constexpr auto as_derived() const -> T const & {
            return static_cast<T const &>(*this);
        }
        constexpr auto as_derived() -> T & { return static_cast<T &>(*this); }

        [[nodiscard]] constexpr auto get(auto f) const {
            return access_t::get(as_derived().data(), f);
        }
        constexpr auto set(auto... fs) -> void {
            access_t::set(as_derived().data(), fs...);
        }
        constexpr auto set() -> void {}

        [[nodiscard]] constexpr auto describe() const {
            return access_t::describe(as_derived().data());
        }
    };

    template <typename> struct owner_t;

    template <typename Span> struct view_t : base<view_t<Span>> {
        using definition_t = message;

        template <detail::storage_like S>
        // NOLINTNEXTLINE(google-explicit-constructor)
        view_t(S const &s) : storage{s} {}

        template <detail::storage_like S, field_value... Vs>
        constexpr explicit view_t(S &s, Vs... vs) : storage{s} {
            this->set(vs...);
        }

        template <typename S>
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr view_t(owner_t<S> const &s) : storage{s.data()} {}

        template <typename S, field_value... Vs>
        explicit constexpr view_t(owner_t<S> &s, Vs... vs) : storage{s.data()} {
            this->set(vs...);
        }

        [[nodiscard]] constexpr auto data() const { return storage; }

      private:
        static_assert((... and Fields::template fits_inside<Span>()),
                      "Fields overflow message storage!");
        Span storage{};
    };
    using const_view_t = view_t<default_const_span_t>;
    using mutable_view_t = view_t<default_span_t>;

    template <typename Storage = default_storage_t>
    struct owner_t : base<owner_t<Storage>> {
        using definition_t = message;
        using storage_t = Storage;

        constexpr owner_t() { this->set(Fields{}...); }

        template <field_value... Vs>
        explicit constexpr owner_t(Vs... vs) : owner_t{} {
            this->set(vs...);
        }

        template <detail::storage_like S, field_value... Vs>
        explicit constexpr owner_t(S const &s, Vs... vs) {
            static_assert(std::is_same_v<typename S::value_type,
                                         typename storage_t::value_type>,
                          "Attempted to construct owning message with "
                          "incompatible storage");

            constexpr auto N = std::min(stdx::ct_capacity_v<S>,
                                        stdx::ct_capacity_v<storage_t>);
            std::copy_n(std::begin(s), N, std::begin(storage));
            this->set(vs...);
        }

        template <detail::storage_like S, field_value... Vs>
        explicit constexpr owner_t(view_t<S> s, Vs... vs)
            : owner_t{s.data(), vs...} {}

        [[nodiscard]] constexpr auto data() { return stdx::span{storage}; }
        [[nodiscard]] constexpr auto data() const {
            return stdx::span{storage};
        }

        [[nodiscard]] constexpr auto as_mutable_view() { return view_t{*this}; }
        [[nodiscard]] constexpr auto as_const_view() const {
            return view_t{*this};
        }

      private:
        static_assert((... and Fields::template fits_inside<storage_t>()),
                      "Fields overflow message storage!");
        storage_t storage{};
    };

    template <detail::storage_like S>
    owner_t(S const &, auto &&...)
        -> owner_t<std::array<typename S::value_type, stdx::ct_capacity_v<S>>>;
    template <field_value... Vs> owner_t(Vs...) -> owner_t<default_storage_t>;
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
};

template <typename T> using owning = typename T::template owner_t<>;
template <typename T> using mutable_view = typename T::mutable_view_t;
template <typename T> using const_view = typename T::const_view_t;
} // namespace msg
