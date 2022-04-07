#include "config_item.hpp"

#include <tuple>
#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_EXTEND_HPP
#define COMPILE_TIME_INIT_BUILD_EXTEND_HPP


namespace cib::detail {
    template<
        typename ExtensionPath,
        typename... Args>
    struct extend : public config_item {
        std::tuple<Args...> args_tuple;

        [[nodiscard]] CIB_CONSTEVAL extend(
            Args const & ... args
        )
            : args_tuple{args...}
        {
            // pass
        }

        template<
            typename TargetBuilder,
            typename Builder>
        [[nodiscard]] CIB_CONSTEVAL auto add(
            path<TargetBuilder> const &,
            Builder const & b
        ) const {
            if constexpr (is_same_v<TargetBuilder, decltype(b.first)>) {
                return std::apply([&](auto const & ... args){
                    return std::pair(b.first, b.second.add(args...));
                }, args_tuple);
            } else {
                return b;
            }
        }

        template<
            typename TargetBuilder,
            typename SubBuilder,
            typename... SubBuilders,
            typename Builder>
        [[nodiscard]] CIB_CONSTEVAL auto add(
            path<TargetBuilder, SubBuilder, SubBuilders...> const &,
            Builder const & b
        ) const {
            if constexpr (is_same_v<TargetBuilder, decltype(b.first)>) {
                return std::apply([&](auto const & ... args){
                    return std::pair(b.first, b.second.template add<SubBuilder, SubBuilders...>(args...));
                }, args_tuple);
            } else {
                return b;
            }
        }

        template<typename Builders, typename... InitArgs>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            Builders const & builders_tuple,
            InitArgs const & ...
        ) const {
            return std::apply([&](auto const & ... builders){
                static_assert(
                    (is_same_v<typename ExtensionPath::First, decltype(builders.first)> + ... + 0) == 1,
                    "Extensions must match exactly one exported builder.");

                return std::make_tuple(add(ExtensionPath{}, builders)...);
            }, builders_tuple);
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_EXTEND_HPP
