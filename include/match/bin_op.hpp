#pragma once

#include <match/concepts.hpp>
#include <match/cost.hpp>
#include <match/negate.hpp>
#include <match/simplify.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

namespace match {
template <template <matcher, matcher> typename Term, stdx::ct_string OpName,
          matcher L, matcher R>
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
        using namespace stdx::literals;
        auto const desc = [&]<matcher M>(M const &m) {
            if constexpr (stdx::is_specialization_of_v<M, Term>) {
                return f(m);
            } else {
                return "("_ctst + f(m) + ")"_ctst;
            }
        };
        return stdx::ct_format<"{} {} {}">(desc(lhs), stdx::cts_t<OpName>{},
                                           desc(rhs));
    }
};

namespace detail {
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
} // namespace detail
} // namespace match
