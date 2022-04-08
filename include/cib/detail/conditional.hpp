#include "compiler.hpp"
#include "config_item.hpp"

#include <tuple>


#ifndef COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP
#define COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP


namespace cib::detail {
    template<
        typename Condition,
        typename... Configs>
    struct conditional : public config_item {
        CIB_CONSTEXPR static Condition condition{};
        detail::config<Configs...> body;

        CIB_CONSTEVAL explicit conditional(
            Condition,
            Configs const & ... configs
        )
           : body{configs...}
        {}

        template<
            typename Builders,
            typename... Args>
        CIB_CONSTEVAL auto init(
            Builders const & builders_tuple,
            Args...
        ) const {
            if constexpr (condition(Args{}...)) {
                return body.init(builders_tuple, Args{}...);
            } else {
                return builders_tuple;
            }
        }

        template<typename... Args>
        CIB_CONSTEVAL auto exports_tuple(Args...) const {
            if constexpr (condition(Args{}...)) {
                return body.exports_tuple(Args{}...);
            } else {
                return std::make_tuple();
            }
        }
    };

    template<
        typename Lhs,
        typename Rhs>
    struct equality {
        CIB_CONSTEXPR static Lhs lhs{};
        CIB_CONSTEXPR static Rhs rhs{};

        template<typename... Args>
        CIB_CONSTEVAL bool operator()(Args const & ... args) const {
            return lhs(args...) == rhs; // FIXME: this assumes the RHS is a literal value
        }
    };

    template<typename MatchType>
    struct arg {
        template<typename Rhs>
        [[nodiscard]] CIB_CONSTEVAL equality<arg<MatchType>, Rhs> operator==(Rhs const &) const {
            return {};
        }

        template<typename... Args>
        CIB_CONSTEVAL auto operator()(Args... args) const {
            return detail::fold_right(std::make_tuple(args...), detail::int_<0>, [=](auto elem, [[maybe_unused]] auto state){
                using ElemType = typename std::remove_cv_t<std::remove_reference_t<decltype(elem)>>::value_type;

                if constexpr (std::is_same_v<ElemType, MatchType>) {
                    return elem;
                } else {
                    return state;
                }
            });
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_CONDITIONAL_HPP
