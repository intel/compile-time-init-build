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
#include <iterator>
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
    } else if constexpr (std::same_as<RelOp, std::equal_to<>>) {
        return std::not_equal_to{};
    } else if constexpr (std::same_as<RelOp, std::not_equal_to<>>) {
        return std::equal_to{};
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
    } else if constexpr (std::same_as<RelOp, std::equal_to<>>) {
        return "=="_sc;
    } else if constexpr (std::same_as<RelOp, std::not_equal_to<>>) {
        return "!="_sc;
    }
}
} // namespace detail

template <typename RelOp, typename Field, typename Field::type ExpectedValue>
struct rel_matcher_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return RelOp{}(extract_field(msg), ExpectedValue);
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} {} 0x{:x}"_sc, Field::name,
                      detail::to_string<RelOp>(),
                      sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) {} 0x{:x}"_sc, Field::name,
                      static_cast<std::uint32_t>(extract_field(msg)),
                      detail::to_string<RelOp>(),
                      sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(match::negate_t,
                                                   rel_matcher_t const &) {
        return rel_matcher_t<decltype(detail::inverse_op<RelOp>()), Field,
                             ExpectedValue>{};
    }

    template <typename Field::type OtherValue>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, rel_matcher_t,
               rel_matcher_t<RelOp, Field, OtherValue>) -> bool {
        return RelOp{}(ExpectedValue, OtherValue);
    }

    template <typename Msg>
    [[nodiscard]] constexpr static auto extract_field(Msg const &msg) {
        if constexpr (stdx::range<Msg>) {
            return Field::extract(msg);
        } else {
            return Field::extract(std::data(msg));
        }
    }
};

template <typename Field, auto ExpectedValue>
using less_than_t = rel_matcher_t<std::less<>, Field, ExpectedValue>;
template <typename Field, auto ExpectedValue>
using less_than_or_equal_to_t =
    rel_matcher_t<std::less_equal<>, Field, ExpectedValue>;
template <typename Field, auto ExpectedValue>
using greater_than_t = rel_matcher_t<std::greater<>, Field, ExpectedValue>;
template <typename Field, auto ExpectedValue>
using greater_than_or_equal_to_t =
    rel_matcher_t<std::greater_equal<>, Field, ExpectedValue>;

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, less_than_or_equal_to_t<Field, X> const &,
           less_than_t<Field, Y> const &) -> bool {
    return X < Y;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, less_than_t<Field, X> const &,
           less_than_or_equal_to_t<Field, Y> const &) -> bool {
    auto inc = decltype(X){};
    return X <= Y + ++inc;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, greater_than_or_equal_to_t<Field, X> const &,
           greater_than_t<Field, Y> const &) -> bool {
    return X > Y;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, greater_than_t<Field, X> const &,
           greater_than_or_equal_to_t<Field, Y> const &) -> bool {
    auto inc = decltype(X){};
    return X + ++inc >= Y;
}

template <typename Field, auto ExpectedValue>
using equal_to_t = rel_matcher_t<std::equal_to<>, Field, ExpectedValue>;

template <typename Field, auto X, typename RelOp, decltype(X) Y>
[[nodiscard]] constexpr auto tag_invoke(match::implies_t,
                                        equal_to_t<Field, X> const &,
                                        rel_matcher_t<RelOp, Field, Y> const &)
    -> bool {
    return RelOp{}(X, Y);
}

template <typename Field, auto X>
constexpr auto tag_invoke(index_terms_t, equal_to_t<Field, X> const &,
                          stdx::callable auto const &f, std::size_t idx)
    -> void {
    f.template operator()<Field>(idx, X);
}

template <typename Field, auto ExpectedValue>
using not_equal_to_t = rel_matcher_t<std::not_equal_to<>, Field, ExpectedValue>;

template <typename Field, auto X>
constexpr auto tag_invoke(index_not_terms_t, not_equal_to_t<Field, X> const &,
                          stdx::callable auto const &f, std::size_t idx,
                          bool = false) -> void {
    f.template operator()<Field>(idx, X);
}

template <typename Field, auto X, typename... Fields>
[[nodiscard]] constexpr auto tag_invoke(remove_terms_t,
                                        equal_to_t<Field, X> const &m,
                                        std::type_identity<Fields>...)
    -> match::matcher auto {
    if constexpr ((std::is_same_v<Field, Fields> or ...)) {
        return match::always;
    } else {
        return m;
    }
}

template <typename Field, auto X, typename... Fields>
[[nodiscard]] constexpr auto tag_invoke(remove_terms_t,
                                        not_equal_to_t<Field, X> const &m,
                                        std::type_identity<Fields>...)
    -> match::matcher auto {
    if constexpr ((std::is_same_v<Field, Fields> or ...)) {
        return match::always;
    } else {
        return m;
    }
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto tag_invoke(match::implies_t,
                                        less_than_t<Field, X> const &,
                                        not_equal_to_t<Field, Y> const &)
    -> bool {
    return X <= Y;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto tag_invoke(match::implies_t,
                                        greater_than_t<Field, X> const &,
                                        not_equal_to_t<Field, Y> const &)
    -> bool {
    return X >= Y;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, less_than_or_equal_to_t<Field, X> const &,
           not_equal_to_t<Field, Y> const &) -> bool {
    return X < Y;
}

template <typename Field, auto X, decltype(X) Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, greater_than_or_equal_to_t<Field, X> const &,
           not_equal_to_t<Field, Y> const &) -> bool {
    return X > Y;
}

template <typename Field, auto... ExpectedValues>
using in_t =
    decltype((equal_to_t<Field, ExpectedValues>{} or ... or match::never));
} // namespace msg
