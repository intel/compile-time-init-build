#pragma once

#include <match/ops.hpp>
#include <msg/field.hpp>
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
};

template <typename Msg, match::matcher M>
constexpr auto make_msg_matcher() -> match::matcher auto {
    if constexpr (std::is_same_v<M, match::always_t>) {
        return M{};
    } else {
        return msg_matcher<Msg, M>{};
    }
}

template <typename NameType, std::uint32_t MaxNumDWords, typename... FieldsT>
struct message_base : public message_data<MaxNumDWords> {
    constexpr static NameType name{};
    constexpr static auto max_num_dwords = MaxNumDWords;
    static_assert((... and (FieldsT::MaxDWordExtent < MaxNumDWords)));
    using FieldTupleType = stdx::tuple<FieldsT...>;

    using matcher_t = decltype(match::all(
        make_msg_matcher<message_base, typename FieldsT::matcher_t>()...));

    // TODO: need a static_assert to check that fields are not overlapping

    template <typename FieldType>
    [[nodiscard]] constexpr static auto is_valid_field() -> bool {
        return (std::is_same_v<typename FieldType::FieldId,
                               typename FieldsT::FieldId> or
                ...);
    }

    constexpr message_base() {
        resize_and_overwrite(
            *this, [](std::uint32_t *, std::size_t) { return MaxNumDWords; });
        (set(FieldsT{}), ...);
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

    template <typename... ArgFields>
    explicit constexpr message_base(ArgFields... argFields) {
        if constexpr ((std::is_integral_v<std::remove_cvref_t<ArgFields>> and
                       ...)) {
            static_assert(sizeof...(ArgFields) <= MaxNumDWords);
            resize_and_overwrite(*this, [&](std::uint32_t *dest, std::size_t) {
                ((*dest++ = static_cast<std::uint32_t>(argFields)), ...);
                return sizeof...(ArgFields);
            });
        } else {
            resize_and_overwrite(*this, [](std::uint32_t *, std::size_t) {
                return MaxNumDWords;
            });
            // TODO: ensure all required fields are set
            // TODO: ensure fields aren't set more than once
            (set(FieldsT{}), ...);
            (set(argFields), ...);
        }
    }

    template <typename FieldType> constexpr void set(FieldType field) {
        static_assert(is_valid_field<FieldType>());
        FieldType::fits_inside(*this);
        field.insert(*this);
    }

    template <typename FieldType> [[nodiscard]] constexpr auto get() const {
        static_assert(is_valid_field<FieldType>());
        FieldType::fits_inside(*this);
        return FieldType::extract(*this);
    }

    [[nodiscard]] constexpr auto describe() const {
        auto const field_descriptions = stdx::transform(
            [&](auto field) {
                using FieldType = decltype(field);
                return FieldType{FieldType::extract(this->data)}.describe();
            },
            FieldTupleType{});

        auto const middle_string = field_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });

        return format("{}({})"_sc, name, middle_string);
    }
};
} // namespace msg
