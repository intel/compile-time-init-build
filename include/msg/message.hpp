#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <msg/field.hpp>
#include <msg/match.hpp>
#include <sc/fwd.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace msg {
namespace detail {
// https://en.cppreference.com/w/cpp/types/void_t
// https://en.cppreference.com/w/Cppreference:FAQ
template <typename, typename = void> constexpr bool is_iterable{};

template <typename T>
constexpr bool is_iterable<T, std::void_t<decltype(std::declval<T>().begin()),
                                          decltype(std::declval<T>().end())>> =
    true;
} // namespace detail

template <std::uint32_t MaxNumDWordsT> struct message_data {
    static constexpr auto MaxNumDWords = MaxNumDWordsT;
    std::uint32_t num_dwords{};
    std::array<std::uint32_t, MaxNumDWords> data{};

    constexpr message_data() = default;

    constexpr message_data(std::initializer_list<std::uint32_t> src)
        : num_dwords{static_cast<std::uint32_t>(src.size())} {
        auto i = std::size_t{};
        for (auto element : src) {
            data[i++] = element;
        }
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const
        -> const std::uint32_t & {
        return data[index];
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index)
        -> std::uint32_t & {
        return data[index];
    }

    template <std::size_t Index>
    [[nodiscard]] constexpr auto get() const -> std::uint32_t const & {
        return data.template get<Index>();
    }

    [[nodiscard]] constexpr auto operator==(message_data const &rhs) const
        -> bool {
        return this->num_dwords == rhs.num_dwords && this->data == rhs.data;
    }

    [[nodiscard]] constexpr auto operator!=(message_data const &rhs) const
        -> bool {
        return !(*this == rhs);
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return num_dwords;
    }
};

template <typename MsgType, typename additional_matcher> struct is_valid_msg_t {
    constexpr static auto matcher =
        match::all(MsgType::match_valid_encoding, additional_matcher{});

    template <typename BaseMsgType>
    [[nodiscard]] constexpr auto operator()(BaseMsgType const &base_msg) const
        -> bool {
        MsgType const msg{base_msg.data};
        return matcher(msg);
    }

    [[nodiscard]] constexpr auto describe() const { return matcher.describe(); }

    template <typename BaseMsgType>
    [[nodiscard]] constexpr auto
    describe_match(BaseMsgType const &base_msg) const {
        MsgType const msg{base_msg.data};
        return matcher.describe_match(msg);
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
    static constexpr NameType name{};
    static constexpr auto NumDWords = NumDWordsT;
    using FieldTupleType = cib::tuple<FieldsT...>;

    template <typename additional_matcherType>
    [[nodiscard]] static constexpr auto match(additional_matcherType) {
        return is_valid_msg_t<This, additional_matcherType>{};
    }

    // TODO: need a static_assert to check that fields are not overlapping

    template <typename FieldType>
    [[nodiscard]] static constexpr auto is_valid_field() -> bool {
        return FieldTupleType{}.fold_right(false, [](auto field, bool isValid) {
            return isValid || std::is_same_v<typename FieldType::FieldId,
                                             typename decltype(field)::FieldId>;
        });
    }

    template <typename T>
    using not_required = std::bool_constant<not std::is_same_v<
        match::always_t<true>, std::decay_t<decltype(T::match_requirements)>>>;

    static constexpr auto match_valid_encoding = []() {
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

    constexpr message_base(std::initializer_list<std::uint32_t> src)
        : message_data<MaxNumDWords>{src} {
        this->num_dwords = NumDWordsT;

        if (src.size() == 0) {
            // default constructor, set default values
            cib::for_each([&](auto field) { set(field); }, FieldTupleType{});
        }
    }

    template <typename... ArgFields>
    explicit constexpr message_base(ArgFields... argFields)
        : message_data<MaxNumDWords>{} {
        this->num_dwords = NumDWordsT;

        if constexpr (sizeof...(argFields) == 0) {
            // default constructor, set default values
            cib::for_each([&](auto field) { set(field); }, FieldTupleType{});
        } else {
            auto const arg_field_tuple = cib::make_tuple(argFields...);
            auto const first_arg = cib::get<0>(arg_field_tuple);

            if constexpr (detail::is_iterable<decltype(first_arg)>) {
                std::copy(std::begin(first_arg), std::end(first_arg),
                          std::begin(this->data));

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
        field.insert(this->data);
    }

    template <typename FieldType> [[nodiscard]] constexpr auto get() const {
        static_assert(is_valid_field<FieldType>());
        FieldType::fits_inside(*this);
        return FieldType::extract(this->data);
    }

    [[nodiscard]] constexpr auto describe() const {
        const auto field_descriptions = cib::transform(
            [&](auto field) {
                using FieldType = decltype(field);
                return FieldType{FieldType::extract(this->data)}.describe();
            },
            FieldTupleType{});

        const auto middle_string = field_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });

        return format("{}({})"_sc, name, middle_string);
    }
};
} // namespace msg
