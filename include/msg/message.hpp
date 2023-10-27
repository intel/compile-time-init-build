#pragma once

#include <match/ops.hpp>
#include <msg/field.hpp>
#include <sc/format.hpp>
#include <sc/fwd.hpp>

#include <stdx/cx_vector.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace msg {
namespace detail {
template <typename T>
concept range = requires(T &t) {
    std::begin(t);
    std::end(t);
};

template <typename T>
using iterator_t = decltype(std::begin(std::declval<T &>()));

template <typename T, typename V>
concept convertible_range_of =
    range<T> and std::convertible_to<std::iter_value_t<iterator_t<T>>, V>;
} // namespace detail

template <std::uint32_t MaxNumDWords>
using message_data = stdx::cx_vector<std::uint32_t, MaxNumDWords>;

template <typename Msg, match::matcher M> struct msg_matcher : M {
    [[nodiscard]] constexpr auto operator()(auto const &base) const -> bool {
        return this->M::operator()(Msg{base});
    }

    [[nodiscard]] constexpr auto describe_match(auto const &base) const {
        return this->M::describe_match(Msg{base});
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(match::negate_t,
                                                   msg_matcher const &self) {
        return msg_matcher<Msg, decltype(match::negate(
                                    static_cast<M const &>(self)))>{};
    }

    template <std::same_as<msg_matcher> Self, match::matcher Other>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, Self &&self, Other const &o) {
        return match::implies(static_cast<M const &>(self), o);
    }

    template <match::matcher Other, std::same_as<msg_matcher> Self>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, Other const &o, Self &&self) {
        return match::implies(o, static_cast<M const &>(self));
    }
};

template <typename Msg, match::matcher M>
constexpr auto make_msg_matcher() -> match::matcher auto {
    if constexpr (std::is_same_v<M, match::always_t>) {
        return M{};
    } else {
        return msg_matcher<Msg, M>{};
    }
}

namespace detail {
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

template <typename NameType, std::uint32_t MaxNumDWords, typename... Fields>
struct message_base : public message_data<MaxNumDWords> {
    constexpr static NameType name{};
    constexpr static auto max_num_dwords = MaxNumDWords;
    static_assert((... and (Fields::max_dword_extent < MaxNumDWords)),
                  "Fields overflow message storage!");

    using matcher_t = decltype(match::all(
        make_msg_matcher<message_base, typename Fields::matcher_t>()...));

    // TODO: need a static_assert to check that fields are not overlapping

    constexpr message_base() {
        resize_and_overwrite(
            *this, [](std::uint32_t *, std::size_t) { return MaxNumDWords; });
        (..., set<Fields>());
    }

    template <detail::convertible_range_of<std::uint32_t> R>
    explicit constexpr message_base(R const &r) {
        resize_and_overwrite(
            *this, [&](std::uint32_t *dest, std::size_t max_size) {
                auto const size = std::min(std::size(r), max_size);
                std::copy_n(std::begin(r), size, dest);
                return size;
            });
    }

    template <field_value... Vs>
    explicit constexpr message_base(Vs... vs) : message_base{} {
        (..., set(vs));
    }

    template <std::integral... Vs> explicit constexpr message_base(Vs... vs) {
        static_assert(sizeof...(Vs) <= MaxNumDWords);
        resize_and_overwrite(*this, [&](std::uint32_t *dest, std::size_t) {
            ((*dest++ = static_cast<std::uint32_t>(vs)), ...);
            return sizeof...(Vs);
        });
    }

    template <typename Field>
    constexpr auto set(typename Field::value_type v) -> void {
        static_assert(is_valid_for_message<Field>(),
                      "Field does not belong to this message!");
        static_assert(Field::template fits_inside<message_base>(),
                      "Field does not fit inside message!");
        Field::insert(*this, v);
    }

    template <typename Field> [[nodiscard]] constexpr auto get() const {
        static_assert(is_valid_for_message<Field>(),
                      "Field does not belong to this message!");
        static_assert(Field::template fits_inside<message_base>(),
                      "Field does not fit inside message!");
        return Field::extract(*this);
    }

    [[nodiscard]] constexpr auto describe() const {
        auto const descs = [&] {
            auto const field_descriptions =
                stdx::tuple{Fields::describe(Fields::extract(*this))...};
            if constexpr (sizeof...(Fields) > 0) {
                return field_descriptions.join(
                    [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });
            } else {
                return ""_sc;
            }
        }();
        return format("{}({})"_sc, name, descs);
    }

  private:
    template <typename Field>
    [[nodiscard]] constexpr static auto is_valid_for_message() -> bool {
        return (std::is_same_v<typename Field::field_id,
                               typename Fields::field_id> or
                ...);
    }

    template <typename Field> constexpr auto set() -> void {
        set<Field>(Field::default_value);
    }

    template <field_value V> constexpr void set(V v) {
        auto const set_by_name = [&]<typename F>() -> bool {
            if constexpr (std::is_same_v<typename V::name_t,
                                         typename F::name_t>) {
                set<F>(static_cast<typename F::value_type>(v.value));
                return true;
            }
            return false;
        };
        (... or set_by_name.template operator()<Fields>());
    }
};
} // namespace msg
