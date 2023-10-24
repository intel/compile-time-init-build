#pragma once

#include <match/ops.hpp>
#include <sc/format.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace msg {
constexpr inline class index_terms_t {
    template <match::matcher M>
    friend constexpr auto tag_invoke(index_terms_t, M const &m,
                                     stdx::callable auto const &f,
                                     std::size_t idx) -> void {
        if constexpr (stdx::is_specialization_of_v<M, match::or_t> or
                      stdx::is_specialization_of_v<M, match::and_t>) {
            tag_invoke(index_terms_t{}, m.lhs, f, idx);
            tag_invoke(index_terms_t{}, m.rhs, f, idx);
        }
        // NOT terms are visited later by index_not_terms
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<index_terms_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} index_terms{};

constexpr inline class index_not_terms_t {
    template <match::matcher M>
    friend constexpr auto tag_invoke(index_not_terms_t, M const &m,
                                     stdx::callable auto const &f,
                                     std::size_t idx, bool negated = false)
        -> void {
        if constexpr (stdx::is_specialization_of_v<M, match::or_t> or
                      stdx::is_specialization_of_v<M, match::and_t>) {
            tag_invoke(index_not_terms_t{}, m.lhs, f, idx, negated);
            tag_invoke(index_not_terms_t{}, m.rhs, f, idx, negated);
        } else if constexpr (stdx::is_specialization_of_v<M, match::not_t>) {
            tag_invoke(index_not_terms_t{}, m.m, f, idx, not negated);
        }
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<index_not_terms_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} index_not_terms{};

constexpr inline class remove_terms_t {
    template <match::matcher M, typename... Fields>
    [[nodiscard]] friend constexpr auto
    tag_invoke(remove_terms_t, M const &m, [[maybe_unused]] Fields... fs)
        -> match::matcher auto {
        if constexpr (stdx::is_specialization_of_v<M, match::or_t>) {
            return tag_invoke(remove_terms_t{}, m.lhs, fs...) or
                   tag_invoke(remove_terms_t{}, m.rhs, fs...);
        } else if constexpr (stdx::is_specialization_of_v<M, match::and_t>) {
            return tag_invoke(remove_terms_t{}, m.lhs, fs...) and
                   tag_invoke(remove_terms_t{}, m.rhs, fs...);
        } else if constexpr (stdx::is_specialization_of_v<M, match::not_t>) {
            // NOTE: we don't apply `not` here because not terms are accounted
            // for in the indexing, so the whole not term becomes true
            return tag_invoke(remove_terms_t{}, m.m, fs...);
        } else {
            return m;
        }
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<remove_terms_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} remove_terms{};

namespace detail {
template <typename RelOp> constexpr auto inverse_op() {
    if constexpr (std::same_as<RelOp, std::less<>>) {
        return std::greater_equal{};
    } else if constexpr (std::same_as<RelOp, std::less_equal<>>) {
        return std::greater{};
    } else if constexpr (std::same_as<RelOp, std::greater<>>) {
        return std::less_equal{};
    } else if constexpr (std::same_as<RelOp, std::greater_equal<>>) {
        return std::less{};
    }
}

template <typename RelOp> constexpr auto to_string() {
    if constexpr (std::same_as<RelOp, std::less<>>) {
        return "<"_sc;
    } else if constexpr (std::same_as<RelOp, std::less_equal<>>) {
        return "<="_sc;
    } else if constexpr (std::same_as<RelOp, std::greater<>>) {
        return ">"_sc;
    } else if constexpr (std::same_as<RelOp, std::greater_equal<>>) {
        return ">="_sc;
    }
}
} // namespace detail

template <typename RelOp, typename FieldType, typename T, T ExpectedValue>
struct rel_matcher_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return RelOp{}(msg.template get<FieldType>(), ExpectedValue);
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} {} 0x{:x}"_sc, FieldType::name,
                      detail::to_string<RelOp>(),
                      sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) {} 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      detail::to_string<RelOp>(),
                      sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(match::negate_t,
                                                   rel_matcher_t const &) {
        return rel_matcher_t<decltype(detail::inverse_op<RelOp>()), FieldType,
                             T, ExpectedValue>{};
    }

    template <T OtherValue>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, rel_matcher_t,
               rel_matcher_t<RelOp, FieldType, T, OtherValue>) -> bool {
        return RelOp{}(ExpectedValue, OtherValue);
    }
};

template <typename FieldType, typename T, T ExpectedValue>
using less_than_t = rel_matcher_t<std::less<>, FieldType, T, ExpectedValue>;
template <typename FieldType, typename T, T ExpectedValue>
using less_than_or_equal_to_t =
    rel_matcher_t<std::less_equal<>, FieldType, T, ExpectedValue>;
template <typename FieldType, typename T, T ExpectedValue>
using greater_than_t =
    rel_matcher_t<std::greater<>, FieldType, T, ExpectedValue>;
template <typename FieldType, typename T, T ExpectedValue>
using greater_than_or_equal_to_t =
    rel_matcher_t<std::greater_equal<>, FieldType, T, ExpectedValue>;

template <typename FieldType, typename T, T X, T Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, less_than_or_equal_to_t<FieldType, T, X> const &,
           less_than_t<FieldType, T, Y> const &) -> bool {
    return X < Y;
}

template <typename FieldType, typename T, T X, T Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, less_than_t<FieldType, T, X> const &,
           less_than_or_equal_to_t<FieldType, T, Y> const &) -> bool {
    return X <= Y + 1;
}

template <typename FieldType, typename T, T X, T Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t,
           greater_than_or_equal_to_t<FieldType, T, X> const &,
           greater_than_t<FieldType, T, Y> const &) -> bool {
    return X > Y;
}

template <typename FieldType, typename T, T X, T Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, greater_than_t<FieldType, T, X> const &,
           greater_than_or_equal_to_t<FieldType, T, Y> const &) -> bool {
    return X + 1 >= Y;
}

template <typename FieldType, typename T, T ExpectedValue> struct equal_to_t {
    using is_matcher = void;

    using field_type = FieldType;
    constexpr static auto expected_values = stdx::make_tuple(ExpectedValue);

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return ExpectedValue == msg.template get<FieldType>();
    }

    [[nodiscard]] constexpr auto describe() const {
        if constexpr (std::is_integral_v<T>) {
            return format("{} == 0x{:x}"_sc, FieldType::name,
                          sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        } else {
            return format("{} == {} (0x{:x})"_sc, FieldType::name,
                          sc::enum_<ExpectedValue>,
                          sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        }
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        if constexpr (std::is_integral_v<T>) {
            return format(
                "{} (0x{:x}) == 0x{:x}"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        } else {
            return format(
                "{} (0x{:x}) == {} (0x{:x})"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::enum_<ExpectedValue>,
                sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        }
    }

  private:
    friend constexpr auto tag_invoke(index_terms_t, equal_to_t const &,
                                     stdx::callable auto const &f,
                                     std::size_t idx) -> void {
        f.template operator()<FieldType>(idx, ExpectedValue);
    }

    friend constexpr auto tag_invoke(index_not_terms_t, equal_to_t const &,
                                     stdx::callable auto const &f,
                                     std::size_t idx, bool negated) -> void {
        if (negated) {
            f.template operator()<FieldType>(idx, ExpectedValue);
        }
    }

    template <typename... Fields>
    [[nodiscard]] friend constexpr auto
    tag_invoke(remove_terms_t, equal_to_t const &m,
               std::type_identity<Fields>...) -> match::matcher auto {
        if constexpr ((std::is_same_v<FieldType, Fields> or ...)) {
            return match::always;
        } else {
            return m;
        }
    }

    template <typename RelOp, T OtherValue>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, equal_to_t,
               rel_matcher_t<RelOp, FieldType, T, OtherValue>) -> bool {
        return RelOp{}(ExpectedValue, OtherValue);
    }

    template <T OtherValue>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, equal_to_t,
               match::not_t<equal_to_t<FieldType, T, OtherValue>>) -> bool {
        return ExpectedValue != OtherValue;
    }
};

template <typename FieldType, typename T, T... ExpectedValues>
using in_t = decltype((equal_to_t<FieldType, T, ExpectedValues>{} or ... or
                       match::never));
} // namespace msg
