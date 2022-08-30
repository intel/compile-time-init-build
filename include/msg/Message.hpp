#pragma once

#include <type_traits>
#include <cstdint>
#include <optional>
#include <algorithm>

#include <container/Array.hpp>
#include <sc/string_constant.hpp>
#include <cib/tuple.hpp>

#include <msg/Match.hpp>
#include <msg/Field.hpp>

namespace detail {
    // https://en.cppreference.com/w/cpp/types/void_t
    // https://en.cppreference.com/w/Cppreference:FAQ
    template <typename, typename = void>
    constexpr bool is_iterable{};

    template <typename T>
    constexpr bool is_iterable<
        T,
        std::void_t<
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
        >
    > = true;
}

template<std::uint32_t MaxNumDWordsT>
struct MessageData {
    static constexpr auto MaxNumDWords = MaxNumDWordsT;
    std::uint32_t numDWords;
    Array<std::uint32_t, MaxNumDWords> data;

    constexpr MessageData()
        : numDWords{0}
        , data{}
    {}

    constexpr MessageData(
        std::initializer_list<std::uint32_t> src
    )
        : numDWords{static_cast<std::uint32_t>(src.size())}
        , data{src}
    {}

    constexpr MessageData(MessageData const & rhs) = default;
    constexpr MessageData & operator=(MessageData const & rhs) = default;
    constexpr MessageData(MessageData && rhs) = default;
    constexpr MessageData & operator=(MessageData && rhs) = default;

    [[nodiscard]] constexpr const std::uint32_t & operator[](std::size_t index) const {
        return data[index];
    }

    [[nodiscard]] constexpr std::uint32_t & operator[](std::size_t index) {
        return data[index];
    }

    template<std::size_t Index>
    [[nodiscard]] constexpr const std::uint32_t get() const {
        return data.template get<Index>();
    }

    [[nodiscard]] constexpr bool operator==(MessageData const & rhs) const {
        return
            this->numDWords == rhs.numDWords &&
            this->data == rhs.data;
    }

    [[nodiscard]] constexpr bool operator!=(MessageData const & rhs) const {
        return !(*this == rhs);
    }

    [[nodiscard]] constexpr std::size_t size() const {
        return numDWords;
    }
};


template<typename MsgType, typename AdditionalMatcher>
struct IsValidMsg {
    constexpr static auto matcher =
        match::all(MsgType::matchValidEncoding, AdditionalMatcher{});

    template<typename BaseMsgType>
    [[nodiscard]] constexpr bool operator()(BaseMsgType const & baseMsg) const {
        MsgType const msg{baseMsg.data};
        return matcher(msg);
    }

    [[nodiscard]] constexpr auto describe() const {
        return matcher.describe();
    }

    template<typename BaseMsgType>
    [[nodiscard]] constexpr auto describeMatch(BaseMsgType const & baseMsg) const {
        MsgType const msg{baseMsg.data};
        return matcher.describeMatch(msg);
    }
};

template<typename MsgType, typename AdditionalMatcher>
[[nodiscard]] constexpr auto isValidMsg(AdditionalMatcher additionalMatcher) {
    return IsValidMsg<MsgType, AdditionalMatcher>{};
}


template<
    typename NameType,
    std::uint32_t MaxNumDWords,
    std::uint32_t NumDWordsT,
    typename... FieldsT>
struct MessageBase : public MessageData<MaxNumDWords> {
    using This = MessageBase<NameType, MaxNumDWords, NumDWordsT, FieldsT...>;
    static constexpr NameType name{};
    static constexpr auto NumDWords = NumDWordsT;
    using FieldTupleType = cib::tuple<FieldsT...>;

    template<typename AdditionalMatcherType>
    [[nodiscard]] static constexpr auto match(AdditionalMatcherType additionalMatcher) {
        return IsValidMsg<This, AdditionalMatcherType>{};
    }

    // TODO: need a static_assert to check that fields are not overlapping

    template<typename FieldType>
    [[nodiscard]] static constexpr bool isValidField() {
        return FieldTupleType{}.fold_right(false, [](auto field, bool isValid){
            return
                isValid ||
                std::is_same_v<
                    typename FieldType::FieldId,
                    typename decltype(field)::FieldId>;
        });
    }

    static constexpr auto matchValidEncoding = [](){
        constexpr auto requiredFields =
            cib::filter(FieldTupleType{}, [](auto field){
                using FieldType = decltype(field);
                return sc::bool_<!std::is_same_v<match::Always<true>, std::decay_t<decltype(FieldType::matchRequirements)>>>;
            });

        if constexpr (requiredFields.size() == 0) {
            return match::always<true>;

        } else {
            return requiredFields.apply([](auto... requiredFieldsPack) {
                return match::all(decltype(requiredFieldsPack)::matchRequirements ...);
            });
        }
    }();

    [[nodiscard]] constexpr bool isValid() const {
        return matchValidEncoding(*this);
    }

    constexpr MessageBase(
        std::initializer_list<std::uint32_t> src
    )
        : MessageData<MaxNumDWords>{src}
    {
        this->numDWords = NumDWordsT;

        if (src.size() == 0) {
            // default constructor, set default values
            FieldTupleType{}.for_each([&](auto field) {
                set(field);
            });
        }
    }

    template<typename... ArgFields>
    explicit constexpr MessageBase(ArgFields... argFields)
        : MessageData<MaxNumDWords>{}
    {
        this->numDWords = NumDWordsT;

        if constexpr (sizeof...(argFields) == 0) {
            // default constructor, set default values
            FieldTupleType{}.for_each([&](auto field) {
                set(field);
            });

        } else {
            auto const argFieldTuple = cib::make_tuple(argFields...);
            auto const firstArg = argFieldTuple.get(cib::index_<0>);

            if constexpr (detail::is_iterable<decltype(firstArg)>) {
                std::copy(std::begin(firstArg), std::end(firstArg), std::begin(this->data));

            } else {
                // TODO: ensure all required fields are set
                // TODO: ensure fields aren't set more than once

                // set default values
                FieldTupleType{}.for_each([&](auto field) {
                    set(field);
                });

                // set specified field values
                argFieldTuple.for_each([&](auto field) {
                    set(field);
                });
            }
        }
    }

    template<typename FieldType>
    constexpr void set(FieldType field) {
        static_assert(isValidField<FieldType>());
        FieldType::fitsInside(*this);
        field.insert(this->data);
    }

    template<typename FieldType>
    [[nodiscard]] constexpr auto get() const {
        static_assert(isValidField<FieldType>());
        FieldType::fitsInside(*this);
        return FieldType::extract(this->data);
    }

    [[nodiscard]] constexpr auto describe() const {
        const auto fieldDescriptions =
            cib::transform(FieldTupleType{}, [&](auto field){
                using FieldType = decltype(field);
                return FieldType{FieldType::extract(this->data)}.describe();
            });

        const auto middleString =
            fieldDescriptions.fold_left([](auto lhs, auto rhs){
                return lhs + ", "_sc + rhs;
            });

        return format("{}({})"_sc, name, middleString);
    }
};
