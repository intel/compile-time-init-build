#pragma once

#include <match/concepts.hpp>
#include <match/cost.hpp>
#include <match/negate.hpp>
#include <match/simplify.hpp>
#include <sc/string_constant.hpp>

#include <stdx/type_traits.hpp>

namespace match {
template <template <matcher, matcher> typename Term, typename OpName, matcher L,
          matcher R>
struct bin_op_t {
    using is_matcher = void;

    using lhs_t = L;
    using rhs_t = R;

    [[no_unique_address]] L lhs;
    [[no_unique_address]] R rhs;

    [[nodiscard]] constexpr auto describe() const {
        return form_description([](auto const &m) { return m.describe(); });
    }

    [[nodiscard]] constexpr auto describe_match(auto const &event) const {
        return form_description(
            [&](auto const &m) { return m.describe_match(event); });
    }

  private:
    [[nodiscard]] constexpr auto form_description(auto const &f) const {
        auto const desc = [&]<matcher M>(M const &m) {
            if constexpr (stdx::is_specialization_of_v<M, Term>) {
                return f(m);
            } else {
                return "("_sc + f(m) + ")"_sc;
            }
        };
        return desc(lhs) + OpName{} + desc(rhs);
    }
};

namespace detail {
template <matcher LHS, matcher RHS, template <matcher, matcher> typename Dual>
[[nodiscard]] CONSTEVAL static auto absorbs() -> bool {
    if constexpr (stdx::is_specialization_of_v<RHS, Dual>) {
        return std::is_same_v<LHS, typename RHS::lhs_t> or
               std::is_same_v<LHS, typename RHS::rhs_t>;
    }
    return false;
}

template <template <matcher, matcher> typename Term,
          template <matcher, matcher> typename Dual, matcher L, matcher R>
[[nodiscard]] constexpr auto de_morgan(L const &l, R const &r) {
    using T = Term<L, R>;
    using D = decltype(negate(Dual{negate(l), negate(r)}));
    if constexpr (cost(std::type_identity<D>{}) <
                  cost(std::type_identity<T>{})) {
        return negate(Dual{negate(l), negate(r)});
    } else {
        return T{l, r};
    }
}

template <matcher A, matcher I, template <matcher, matcher> typename Dual,
          template <matcher, matcher> typename Term, matcher L, matcher R>
[[nodiscard]] constexpr auto simplify_bin_op(Term<L, R> const &m) {
    auto l = simplify(m.lhs);
    auto r = simplify(m.rhs);
    using LS = decltype(l);
    using RS = decltype(r);

    if constexpr (std::is_same_v<A, LS> or std::is_same_v<A, RS> or
                  std::is_same_v<LS, decltype(negate(std::declval<RS>()))>) {
        return A{};
    } else if constexpr (std::is_same_v<LS, RS> or std::is_same_v<I, RS> or
                         absorbs<LS, RS, Dual>()) {
        return l;
    } else if constexpr (std::is_same_v<I, LS> or absorbs<RS, LS, Dual>()) {
        return r;
    } else {
        return de_morgan<Term, Dual>(l, r);
    }
}
} // namespace detail
} // namespace match
