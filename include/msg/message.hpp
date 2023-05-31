#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <container/vector.hpp>
#include <msg/field.hpp>
#include <msg/match.hpp>
#include <sc/fwd.hpp>

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace msg {
namespace detail {
template <typename T>
concept range = requires(T &t) {
                    std::begin(t);
                    std::end(t);
                };
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

template <typename NameType, std::uint32_t MaxNumDWords,
          std::uint32_t NumDWordsT, typename... FieldsT>
struct message_base : public message_data<MaxNumDWords> {
    using This = message_base<NameType, MaxNumDWords, NumDWordsT, FieldsT...>;
    constexpr static NameType name{};
    constexpr static auto NumDWords = NumDWordsT;
    using FieldTupleType = cib::tuple<FieldsT...>;

    template <typename additional_matcherType>
    [[nodiscard]] constexpr static auto match(additional_matcherType) {
        return is_valid_msg_t<This, additional_matcherType>{};
    }

    // TODO: need a static_assert to check that fields are not overlapping

    template <typename FieldType>
    [[nodiscard]] constexpr static auto is_valid_field() -> bool {
        return FieldTupleType{}.fold_right(false, [](auto field, bool isValid) {
            return isValid || std::is_same_v<typename FieldType::FieldId,
                                             typename decltype(field)::FieldId>;
        });
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

    constexpr message_base(std::initializer_list<std::uint32_t> src) {
        this->current_size = NumDWordsT;

        if (src.size() == 0) {
            // default constructor, set default values
            cib::for_each([&](auto field) { set(field); }, FieldTupleType{});
        } else {
            std::copy(std::begin(src), std::end(src), std::begin(*this));
        }
    }

    template <typename... ArgFields>
    explicit constexpr message_base(ArgFields... argFields) {
        this->current_size = NumDWordsT;

        if constexpr (sizeof...(argFields) == 0) {
            // default constructor, set default values
            cib::for_each([&](auto field) { set(field); }, FieldTupleType{});
        } else {
            auto const arg_field_tuple = cib::make_tuple(argFields...);
            auto const first_arg = cib::get<0>(arg_field_tuple);

            if constexpr (detail::range<decltype(first_arg)>) {
                std::copy(std::begin(first_arg), std::end(first_arg),
                          std::begin(*this));

            } else {
                // TODO: ensure all required fields are set
                // TODO: ensure fields aren't set more than once

                // set default values
                cib::for_each([&](auto field) { set(field); },
                              FieldTupleType{});

                // set specified field values
                cib::for_each([&](auto field) { set(field); }, arg_field_tuple);
            }
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
