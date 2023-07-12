#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <container/vector.hpp>
#include <msg/field.hpp>
#include <msg/match.hpp>
#include <sc/fwd.hpp>

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
using message_data = cib::vector<std::uint32_t, MaxNumDWords>;

template <typename MsgType, typename additional_matcher> struct is_valid_msg_t {
    constexpr static auto matcher =
        match::all(MsgType::match_valid_encoding, additional_matcher{});

    template <typename BaseMsgType>
    [[nodiscard]] constexpr auto operator()(BaseMsgType const &base_msg) const
        -> bool {
        return matcher(MsgType{base_msg});
    }

    [[nodiscard]] constexpr auto describe() const { return matcher.describe(); }

    template <typename BaseMsgType>
    [[nodiscard]] constexpr auto
    describe_match(BaseMsgType const &base_msg) const {
        return matcher.describe_match(MsgType{base_msg});
    }
};

template <typename MsgType, typename AdditionalMatcher>
[[nodiscard]] constexpr auto is_valid_msg(AdditionalMatcher) {
    return is_valid_msg_t<MsgType, AdditionalMatcher>{};
}

template <typename NameType, std::uint32_t MaxNumDWords, typename... FieldsT>
struct message_base : public message_data<MaxNumDWords> {
    constexpr static NameType name{};
    constexpr static auto max_num_dwords = MaxNumDWords;
    static_assert((... and (FieldsT::MaxDWordExtent < MaxNumDWords)));
    using FieldTupleType = cib::tuple<FieldsT...>;

    template <typename additional_matcherType>
    [[nodiscard]] constexpr static auto match(additional_matcherType) {
        return is_valid_msg_t<message_base, additional_matcherType>{};
    }

    // TODO: need a static_assert to check that fields are not overlapping

    template <typename FieldType>
    [[nodiscard]] constexpr static auto is_valid_field() -> bool {
        return (std::is_same_v<typename FieldType::FieldId,
                               typename FieldsT::FieldId> or
                ...);
    }

    template <typename T>
    using not_required = std::bool_constant<not std::is_same_v<
        match::always_t<true>, std::decay_t<decltype(T::match_requirements)>>>;

    constexpr static auto match_valid_encoding = []() {
        constexpr auto required_fields =
            cib::filter<not_required>(FieldTupleType{});
        if constexpr (required_fields.size() == 0) {
            return match::always<true>;
        } else {
            return required_fields.apply([](auto... required_fields_pack) {
                return match::all(
                    decltype(required_fields_pack)::match_requirements...);
            });
        }
    }();

    [[nodiscard]] constexpr auto isValid() const -> bool {
        return match_valid_encoding(*this);
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
        auto const field_descriptions = cib::transform(
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
