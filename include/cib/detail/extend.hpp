#include "config_item.hpp"
#include "indexed_tuple.hpp"
#include "ordered_set.hpp"

#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_EXTEND_HPP
#define COMPILE_TIME_INIT_BUILD_EXTEND_HPP


namespace cib::detail {
    template<
        typename ExtensionPath,
        typename... Args>
    struct extend : public config_item {
        ordered_set<Args...> args_tuple;

        CIB_CONSTEVAL explicit extend(
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
            if constexpr (is_same_v<TargetBuilder, typename std::remove_cv_t<std::remove_reference_t<decltype(b)>>::Service>) {
                return apply([&](auto const & ... args){
                    return service_entry <
                       typename std::remove_cv_t<std::remove_reference_t<decltype(b)>>::Service,
                        decltype(b.builder.add(args...))>{b.builder.add(args...)};
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
            if constexpr (is_same_v<TargetBuilder, typename std::remove_cv_t<std::remove_reference_t<decltype(b)>>::Service>) {
                return apply([&](auto const & ... args){
                    return service_entry <
                       typename std::remove_cv_t<std::remove_reference_t<decltype(b)>>::Service,
                        decltype(b.builder.template add<SubBuilder, SubBuilders...>(args...))>{b.builder.template add<SubBuilder, SubBuilders...>(args...)};
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
            return apply([&](auto const & ... builders){
                static_assert(
                    (is_same_v<typename ExtensionPath::First, typename std::remove_cv_t<std::remove_reference_t<decltype(builders)>>::Service> + ... + 0) > 0,
                    "Extension didn't match any service");

                static_assert(
                    (is_same_v<typename ExtensionPath::First, typename std::remove_cv_t<std::remove_reference_t<decltype(builders)>>::Service> + ... + 0) <= 1,
                    "Extension matched more than 1 service");

                return detail::make_indexed_tuple(detail::index_metafunc_<extract_service_tag>{}, add(ExtensionPath{}, builders)...);
            }, builders_tuple);
        }


    };
}


#endif //COMPILE_TIME_INIT_BUILD_EXTEND_HPP
