#include <cib/detail/compiler.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/type_list.hpp>
#include <cib/tuple.hpp>

#ifndef COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP
#define COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP

namespace cib::detail {
template <typename Condition, typename... Configs>
struct conditional : public config_item {
    CIB_CONSTEXPR static Condition condition{};
    detail::config<detail::args<>, Configs...> body;

    CIB_CONSTEVAL explicit conditional(Condition, Configs const &...configs)
        : body{{}, configs...} {}

    template <typename... Args>
    [[nodiscard]] CIB_CONSTEXPR auto extends_tuple(Args const &...) const {
        if constexpr (condition(Args{}...)) {
            return body.extends_tuple(Args{}...);
        } else {
            return cib::tuple<>{};
        }
    }
};

template <typename Lhs, typename Rhs> struct equality {
    CIB_CONSTEXPR static Lhs lhs{};
    CIB_CONSTEXPR static Rhs rhs{};

    template <typename... Args>
    CIB_CONSTEXPR auto operator()(Args const &...args) const -> bool {
        return lhs(args...) ==
               rhs; // FIXME: this assumes the RHS is a literal value
    }
};

template <typename MatchType> struct arg {
    template <typename Rhs>
    [[nodiscard]] CIB_CONSTEVAL auto operator==(Rhs const &) const
        -> equality<arg<MatchType>, Rhs> {
        return {};
    }

    template <typename... Args>
    CIB_CONSTEXPR auto operator()(Args... args) const {
        return cib::make_tuple(self_type_index, args...)
            .fold_right(
                detail::int_<0>, [=](auto elem, [[maybe_unused]] auto state) {
                    using ElemType = typename std::remove_cv_t<
                        std::remove_reference_t<decltype(elem)>>::value_type;

                    if constexpr (std::is_same_v<ElemType, MatchType>) {
                        return elem;
                    } else {
                        return state;
                    }
                });
    }
};
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP
