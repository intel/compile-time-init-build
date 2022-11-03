#pragma once

#include <cib/tuple.hpp>
#include <container/Array.hpp>
#include <msg/field.hpp>
#include <msg/match.hpp>
#include <sc/string_constant.hpp>

#include <algorithm>
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
    std::uint32_t num_dwords;
    Array<std::uint32_t, MaxNumDWords> data;

    constexpr message_data() : num_dwords{0}, data{} {}

    constexpr message_data(std::initializer_list<std::uint32_t> src)
        : num_dwords{static_cast<std::uint32_t>(src.size())}, data{src} {}

    constexpr message_data(message_data const &rhs) = default;
    constexpr message_data &operator=(message_data const &rhs) = default;
    constexpr message_data(message_data &&rhs) = default;
    constexpr message_data &operator=(message_data &&rhs) = default;

    [[nodiscard]] constexpr const std::uint32_t &
    operator[](std::size_t index) const {
        return data[index];
    }

    [[nodiscard]] constexpr std::uint32_t &operator[](std::size_t index) {
        return data[index];
    }

    template <std::size_t Index>
    [[nodiscard]] constexpr const std::uint32_t &get() const {
        return data.template get<Index>();
    }

    [[nodiscard]] constexpr bool operator==(message_data const &rhs) const {
        return this->num_dwords == rhs.num_dwords && this->data == rhs.data;
    }

    [[nodiscard]] constexpr bool operator!=(message_data const &rhs) const {
        return !(*this == rhs);
    }

    [[nodiscard]] constexpr std::size_t size() const { return num_dwords; }
};

template <typename MsgType, typename additional_matcher> struct is_valid_msg_t {
    constexpr static auto matcher =
        match::all(MsgType::match_valid_encoding, additional_matcher{});

    template <typename BaseMsgType>
    [[nodiscard]] constexpr bool operator()(BaseMsgType const &base_msg) const {
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
    [[nodiscard]] static constexpr bool is_valid_field() {
        return FieldTupleType{}.fold_right(false, [](auto field, bool isValid) {
            return isValid || std::is_same_v<typename FieldType::FieldId,
                                             typename decltype(field)::FieldId>;
        });
    }

    static constexpr auto match_valid_encoding = []() {
        constexpr auto required_fields =
            cib::filter(FieldTupleType{}, [](auto field) {
                using FieldType = decltype(field);
                return sc::bool_<!std::is_same_v<
                    match::always_t<true>,
                    std::decay_t<decltype(FieldType::match_requirements)>>>;
            });

        if constexpr (required_fields.size() == 0) {
            return match::always<true>;

        } else {
            return required_fields.apply([](auto... required_fields_pack) {
                return match::all(
                    decltype(required_fields_pack)::match_requirements...);
            });
        }
    }();

    [[nodiscard]] constexpr bool isValid() const {
        return match_valid_encoding(*this);
    }

    constexpr message_base(std::initializer_list<std::uint32_t> src)
        : message_data<MaxNumDWords>{src} {
        this->num_dwords = NumDWordsT;

        if (src.size() == 0) {
            // default constructor, set default values
            FieldTupleType{}.for_each([&](auto field) { set(field); });
        }
    }

    template <typename... ArgFields>
    explicit constexpr message_base(ArgFields... argFields)
        : message_data<MaxNumDWords>{} {
        this->num_dwords = NumDWordsT;

        if constexpr (sizeof...(argFields) == 0) {
            // default constructor, set default values
            FieldTupleType{}.for_each([&](auto field) { set(field); });

        } else {
            auto const arg_field_tuple = cib::make_tuple(argFields...);
            auto const first_arg = arg_field_tuple.get(cib::index_<0>);

            if constexpr (detail::is_iterable<decltype(first_arg)>) {
                std::copy(std::begin(first_arg), std::end(first_arg),
                          std::begin(this->data));

            } else {
                // TODO: ensure all required fields are set
                // TODO: ensure fields aren't set more than once

                // set default values
                FieldTupleType{}.for_each([&](auto field) { set(field); });

                // set specified field values
                arg_field_tuple.for_each([&](auto field) { set(field); });
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
        const auto field_descriptions =
            cib::transform(FieldTupleType{}, [&](auto field) {
                using FieldType = decltype(field);
                return FieldType{FieldType::extract(this->data)}.describe();
            });

        const auto middle_string = field_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + ", "_sc + rhs; });

        return format("{}({})"_sc, name, middle_string);
    }
};
} // namespace msg
